#ifndef UTILS_HPP
#define UTILS_HPP

#include<string>
#include<algorithm>

/*!
 * @brief Remove caracteres do início de uma string.
 *
 * Esta função cria uma cópia da string recebida e remove todos
 * os caracteres iniciais que pertençam ao conjunto especificado
 * em `t`.
 *
 * Por padrão, são removidos espaços em branco, tabulações e
 * caracteres de quebra de linha.
 *
 * @param s String a ser processada.
 * @param t Conjunto de caracteres a serem removidos.
 *
 * @return Uma nova string sem os caracteres indesejados à esquerda.
 */
inline std::string ltrim(const std::string& s, const char* t = " \t\n\r\f\v") {
  std::string clone{ s };
  clone.erase(0, clone.find_first_not_of(t));
  return clone;
}

/*!
 * @brief Remove caracteres do final de uma string.
 *
 * Esta função cria uma cópia da string recebida e remove todos
 * os caracteres finais que pertençam ao conjunto especificado
 * em `t`.
 *
 * Por padrão, são removidos espaços em branco, tabulações e
 * caracteres de quebra de linha.
 *
 * @param s String a ser processada.
 * @param t Conjunto de caracteres a serem removidos.
 *
 * @return Uma nova string sem os caracteres indesejados à direita.
 */
inline std::string rtrim(const std::string& s, const char* t = " \t\n\r\f\v") {
  std::string clone{ s };
  reverse(clone.begin(), clone.end());
  clone.erase(0, clone.find_first_not_of(t));
  reverse(clone.begin(), clone.end());
  return clone;
}

/*!
 * @brief Remove caracteres do início e do final de uma string.
 *
 * Esta função combina as operações realizadas por `ltrim()` e
 * `rtrim()`, removendo caracteres indesejados de ambas as
 * extremidades da string.
 *
 * Por padrão, são removidos espaços em branco, tabulações e
 * caracteres de quebra de linha.
 *
 * @param s String a ser processada.
 * @param t Conjunto de caracteres a serem removidos.
 *
 * @return Uma nova string sem os caracteres especificados nas
 * extremidades.
 */
inline std::string trim(const std::string& s, const char* t = " \t\n\r\f\v") {
  return ltrim(rtrim(s, t), t);
}

/*!
 * @brief Converte todos os caracteres de uma string para minúsculo.
 *
 * Esta função cria uma cópia da string recebida e converte cada
 * caractere para sua representação em letra minúscula utilizando
 * `std::tolower()`.
 *
 * A string original não é modificada.
 *
 * @param s String a ser convertida.
 *
 * @return Uma nova string contendo apenas caracteres minúsculos.
 */
inline std::string to_lower(const std::string& s) {
    std::string clone{ s };

    for(size_t i = 0; i < clone.size(); i++){
      clone[i] = std::tolower((unsigned char) clone[i]);
    }

    return clone;
}

#endif