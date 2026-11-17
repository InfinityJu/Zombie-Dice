#pragma once

#include <cmath>
#include <functional>
#include <memory>
#include <random>
#include <stdexcept>
#include <unordered_map>
#include <utility>
#include <vector>

/* 
   Os nós de decisão binária, roteamento(switch), aleatórios e leaf foram baseados nas ideias do livro:
   MILLINGTON, Ian; FUNGE, John. Artificial Intelligence for Games. 2. ed. 2009.

   O nó de decisão binária funciona como um nó que toma decisões determinísticas, basicamente, 
   compara valores do contexto e decide entre o caminho a e o caminho b.

   O nó de roteamente funciona do mesmo modo que o nó de decisão binária, mas funciona com uma quantidade n de caminhos.
   No caso de serem necessárias várias comparações para o caminho ser decidido, por exemplo: o tipo do bot, onde o nó simplesmente
   olha o tipo e aponta em O(1) o caminho certo para esse bot. A alternativa seria usar vários nós de decisão binária, que fariam O(n)
   comparações.

   O nó de chance é simples de ser explicado. Ele é citado no livro mencionado acima como um meio para que a IA de um jogo não pareça
   extremamente robótica, como se fosse apenas um amontoado de if-elses. Esse nó pode funcionar de dois modos. Um de chance fixa, isto é, cada
   caminha tem uma chance fixa de ser rolado. Útil para o coin flip do bot gambler. 
   Ou pode funcionar com chance dinâmica, esse nó funciona usando o contexto do jogo para calcular a chance dos caminhos possíveis, por exemplo, um bot que é mais inteligente tem mais chance de arriscar
   quando ele não toma nenhum tiro, mas ainda assim, tem chance de simplesmente tomar a decisão mais conservadora.

   O nó leaf indica uma decisão tomada, ou seja, significa que a busca na árvore chegou ao fim e o bot escolheu uma decisão, no caso do zombie dice
   esse nó pode ser Hold ou Roll. 

   O nó de decisão baseado em probabilidade é uma especialização do nó FloatDecision que é mencionado no livro acima.
   Este nó é uma especialização do nó citado, funcionando para tomar decisões determinísticas sobre o jogo. A sua ideia se dá
   em calcular as probabilidades do bot morrer ou viver. Esse valor será comparado com um "cutoff", um valor mínimo que diz o quão aceitável é 
   se arriscar, um valor mais baixo aceita se arriscar mais, o cutoff, em um bot muito inteligente, esse valor não será fixo e será baseado na ideia de utilidade apresentada no livro:
   RUSSELL, Stuart; NORVIG, Peter. Artificial Intelligence: A Modern Approach. 4. ed. 2020

   O livro mencionado logo acima apresenta árvores como a MiniMax e a ExpectMiniMax. Que apesar de serem ótimas bases de pensamento sobre como resolver o problema no zombie dice,
   não são capazes de integrar um bom bot para esse jogo, visto que a MiniMax não trabalha com jogos estocásticos, Zombie Dice usa a rolagem de dados para dar pontos.
   Já o ExpectMiniMax que apesar de trabalhar com chance e jogos estocásticos não funciona bem para jogos com muitos jogadores, além disso, o passo Min desse algoritmo, que
   busca reduzir a utilidade do oponente, é simplesmente negado, pois no Zombie Dice não há um modo de interferir no jogo do oponente, durante o turno de um jogador ele apenas pode mudar o próprio jogo.
   
*/

namespace pdt {

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

  namespace helpers {

    template <typename Context, typename Result, typename Pred>
    NodePtr<Context, Result> decide(Pred&& pred, NodePtr<Context, Result> onTrue, NodePtr<Context, Result> onFalse) {
        return std::make_unique<BinaryNode<Context, Result>>(std::forward<Pred>(pred), std::move(onTrue), std::move(onFalse));
    }

    template <typename Context, typename Result, typename Key, typename Selector>
    std::unique_ptr<SwitchNode<Context, Result, Key>> switchOn(Selector&& selector, NodePtr<Context, Result> fallback = nullptr) {
        return std::make_unique<SwitchNode<Context, Result, Key>>(std::forward<Selector>(selector), std::move(fallback));
    }

    template <typename Context, typename Result>
    std::unique_ptr<ChanceNode<Context, Result>> chance() {
        return std::make_unique<ChanceNode<Context, Result>>();
    }

    template <typename Context, typename Result, typename Rng = std::mt19937, typename Provider>
    std::unique_ptr<ChanceNode<Context, Result, Rng>> chanceWithRng(Provider&& rngProvider) {
        return std::make_unique<ChanceNode<Context, Result, Rng>>(std::forward<Provider>(rngProvider));
    }

    template <typename Context, typename Result, typename ProbFn, typename ThresholdOrFn>
    NodePtr<Context, Result> probThreshold(ProbFn&& probFn, ThresholdOrFn&& threshold, 
                                           NodePtr<Context, Result> onAboveOrEqual, NodePtr<Context, Result> onBelow) {
        return std::make_unique<ProbabilityThresholdNode<Context, Result>>(std::forward<ProbFn>(probFn), std::forward<ThresholdOrFn>(threshold),
                                                                           std::move(onAboveOrEqual), std::move(onBelow));
    }

    template <typename Context, typename Result, typename ValueOrFn>
    NodePtr<Context, Result> leaf(ValueOrFn&& val) {
        return std::make_unique<LeafNode<Context, Result>>(std::forward<ValueOrFn>(val));
    }
  }
}
