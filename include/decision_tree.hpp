#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <random>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

namespace AiTree {

  inline std::mt19937& defaultRng() {
    std::mt19937 generator(std::random_device{}());
    return generator;
  }

  //Nó base, será pai dos nós de decisão
  //Evaluate é a função que decide qual será o nó escolhido
  template <typename Context, typename Result>
  class Node {
    public:
        virtual ~Node() = default;
        virtual Result evaluate(Context& ctx) const = 0;     
  };

  template <typename Context, typename Result>
  using NodePtr = std::unique_ptr<Node<Context, Result>>;

  //Nó de decisão binária.
  template <typename Context, typename Result>
  class BinaryNode : public Node<Context, Result>{
    std::function<bool(const Context&)> predicate;
    NodePtr<Context, Result> trueBranch;
    NodePtr<Context, Result> falseBranch;

    public:
      BinaryNode(std::function<bool(const Context&)> pred, NodePtr<Context, Result> onTrue, NodePtr<Context, Result> onFalse) : predicate(std::move(pred)), trueBranch(std::move(onTrue)), falseBranch(std::move(onFalse))
      {
        if (!predicate or !trueBranch or !falseBranch) 
          throw std::invalid_argument("Error! BinaryNode must have a valid predicate, true branch and false branch");       
      }

      Result evaluate(Context& ctx) const override {
        return predicate(ctx) ? trueBranch->evaluate(ctx) : falseBranch->evaluate(ctx);
      }
  };


  // Nó roteador, serve para substituir grandes cadeias de nós binários
  template <typename Context, typename Result, typename Key>
  class SwitchNode : public Node<Context, Result> {
    std::function<Key(const Context&)> keySelector;
    std::unordered_map<Key, NodePtr<Context, Result>> branches;
    NodePtr<Context, Result> defaultBranch;

    public:
      explicit SwitchNode(std::function<Key(const Context&)> selector, NodePtr<Context, Result> fallback)
        : keySelector(selector), defaultBranch(fallback) {

        if(!selector)
          throw std::invalid_argument("Error! SwitchNode must have a valid selector");  
      }

      SwitchNode& addCase(Key key, NodePtr<Context, Result> branch) {
        if (!branch) 
          throw std::invalid_argument("Error! SwitchNode: null branch added");

        branches[std::move(key)] = std::move(branch);

        return *this;
      }

      Result evaluate(Context& ctx) const override {
        Key key = keySelector(ctx);
        auto it = branches.find(key);

        if (it != branches.end()) return it->second->evaluate(ctx);

        if(defaultBranch) return defaultBranch->second->evaluate(ctx);

        throw std::runtime_error("Error! SwitchNode did not find a valid branch nor a default is defined");
      }  
    
  };

  template<typename Context, typename Result, typename Rng = std::mt19937>
  class ChanceNode : public Node<Context, Result> {
    private:

      struct WeightedBranch {
        std::function<double(const Context&)> weightFn;
        NodePtr<Context, Result> node;
      };
      
      std::function<Rng&(Context&)> rngProvider;
      std::vector<WeightedBranch> branches;

    public:
      //A diferença entre os dois construtores abaixo é que o primeiro usa um rng default, o segundo usa um rng que pode ser fornecido de fora.
      
      ChanceNode() : rngProvider([](Context&) -> Rng& {return defaultRng(); }) {};

      explicit ChanceNode(std::function<Rng&(Context&)> customRngProvider) : rngProvider(std::move(customRngProvider)) {
        if (!rngProvider) {
            throw std::invalid_argument("Error! ChanceNode requires a valid rng");
        }
      }

      //O primeiro dos add's adiciona um branch com peso estatístico fixo, i.e.: não lê informações do ambiente.
      //O segundo: adiciona um branch com peso estatístico variável, ou seja, leva em conta o ambiente.

      ChanceNode& addBranch(double staticWeight, NodePtr<Context, Result> branch) {
        if (std::isnan(staticWeight) || staticWeight < 0.0) {
            throw std::invalid_argument("Error! ChanceNode must receive a statistic weight >= 0.0");
        }
        return addDynamicBranch(
            [staticWeight](const Context&) { return staticWeight; },
            std::move(branch)
        );
      }

      ChanceNode& addDynamicBranch(std::function<double(const Context&)> weightFn,
                                 NodePtr<Context, Result> branch) {
        if (!weightFn || !branch) {
            throw std::invalid_argument("Error! ChanceNode must receive a valid branch and weight function");
        }
        branches.push_back(WeightedBranch{std::move(weightFn), std::move(branch)});
        return *this;
      }
            
      Result evaluate(Context& ctx) const override {
        if (branches.empty()) {
            throw std::runtime_error("Error! ChanceNode does not have any branch added to it");
        }

        std::vector<double> weights;
        weights.reserve(branches.size());
        double totalWeight = 0.0;

        for (const auto& b : branches) {
            double w = b.weightFn(ctx);
            if (std::isnan(w) || w < 0.0) {
                throw std::domain_error("Error! ChanceNode has a branch with NaN or negative weight");
            }
            weights.push_back(w);
            totalWeight += w;
        }

        if (totalWeight <= 0.0) 
            throw std::runtime_error("Error! ChanceNode has weight sum < 0");
        

        std::discrete_distribution<size_t> distribution(weights.begin(), weights.end());
        size_t selectedIndex = distribution(rngProvider(ctx));
        return branches[selectedIndex].node->evaluate(ctx);
    }
    

    
  };

  template <typename Context, typename Result>
  class ProbabilityThresholdNode : public Node<Context, Result> {
    private:
      std::function<double(const Context&)> probabilityFn;
      std::function<double(const Context&)> thresholdFn;
      NodePtr<Context, Result> aboveOrEqualBranch;
      NodePtr<Context, Result> belowBranch;

    public:

      //A diferença entre os contrutores abaixo é dada pelo seu limiar, no primeiro, o limiar é fixo, no segundo ele muda de acordo com o contexto
      ProbabilityThresholdNode(std::function<double(const Context&)> probFn, double fixedThreshold,
                               NodePtr<Context, Result> onAboveOrEqual, NodePtr<Context, Result> onBelow)
                               : ProbabilityThresholdNode(std::move(probFn), [fixedThreshold](const Context&) { return fixedThreshold; },
                                                          std::move(onAboveOrEqual), std::move(onBelow)) {}

      
      ProbabilityThresholdNode(std::function<double(const Context&)> probFn, std::function<double(const Context&)> dynamicThresholdFn, 
                               NodePtr<Context, Result> onAboveOrEqual, NodePtr<Context, Result> onBelow)
                               : probabilityFn(std::move(probFn)), thresholdFn(std::move(dynamicThresholdFn)),
                                               aboveOrEqualBranch(std::move(onAboveOrEqual)), belowBranch(std::move(onBelow)) 
      {
          if (!probabilityFn || !thresholdFn || !aboveOrEqualBranch || !belowBranch) 
              throw std::invalid_argument("Error! ProbabilityThresholdNode has received invalid arguments");
          
      }

      Result evaluate(Context& ctx) const override {
          double prob = probabilityFn(ctx);
          double threshold = thresholdFn(ctx);

          if (std::isnan(prob) || prob < 0.0 || prob > 1.0) 
              throw std::domain_error("Error! ProbabilityThresholdNode receive a probability that is not between 0.0 and 1.0");
          

          return (prob >= threshold) ? aboveOrEqualBranch->evaluate(ctx) : belowBranch->evaluate(ctx);
      }
  };


  template <typename Context, typename Result>
  class LeafNode : public Node<Context, Result> {
      std::function<Result(Context&)> action;

  public:
      explicit LeafNode(std::function<Result(Context&)> fn) : action(std::move(fn)) {
          if (!action) 
              throw std::invalid_argument("Error! LeafNode requires a valid function");
          
      }

      explicit LeafNode(Result constantResult) : action([res = std::move(constantResult)](Context&) { return res; }) {}

      Result evaluate(Context& ctx) const override {
          return action(ctx);
      }
  };

  //TODO fazer funções helper. Olhando o plano é possível ver quais faltam
  
}
