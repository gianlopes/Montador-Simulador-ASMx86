/* Gianlucas Dos Santos Lopes - 180041991 */

#include <iostream>
#include <string>
#include <fstream>
#include <list>
#include <algorithm> // std::transform
#include <regex>
#include <vector>
#include <map>
#include <tuple>
#include "stringUtils.hpp"
#include "montador.hpp"
#include "trataErros.hpp"

int main(int, char *argv[])
{
    /* File IO */
    std::string path{argv[1]};
    std::ifstream inputFile{};
    inputFile.open(path);
    if(!inputFile.is_open()){
        std::cout << "Falha ao abrir o arquivo de entrada" << std::endl;
        exit(1);
    }
    std::ofstream outputFile{};
    outputFile.open(path.replace(path.end() - 4, path.end(), ".obj"));
    if(!outputFile.is_open()){
        std::cout << "Falha ao abrir o arquivo de saída" << std::endl;
        exit(1);
    }

    /* Lista para armazenar linhas do arquivo*/
    std::list<tipoLinha> lista{};
    std::list<tipoLinha> listaData{};

    /* Aux */
    tipoLinha linha;
    std::string s;
    std::smatch m;
    std::vector<std::string> tokens;

    TrataErros::start();
    int contador_linha = 0;

    /*
    Passagem 0
    Salva as linhas do arquivo asm em uma lista de linhas, onde as linhas da seção de dados vem depois
    da seção de texto. Cada linha contém um rótulo, uma instrução, um campo de operandos (separados por vígulas
    sem espaços) e um número da linha original no arquivo. Os campos podem ser vazios "".
    */
    while (std::getline(inputFile, s)) // Lê uma linha do arquivo
    {
        contador_linha++;
        s = std::regex_replace(s, std::regex{";.*"}, "");     // Desconsiderando comentários
        s = std::regex_replace(s, std::regex{"[\\s]+"}, " "); // Desconsiderando espaços desnecéssarios
        trim(s);                                              // Corta espaços no começo o no final da linha
        s = strToLower(s);

        if (s == "section data")
            continue; // Assume que a sessão de dados sempre vem primeiro
        if (s == "section text")
        {
            listaData = std::move(lista); // Move a sessão de dados para outra lista
            continue;
        }
        if (s == " " || s == "") // Se a linha estiver vazia, é desconsiderada.
            continue;

        if (std::regex_search(s, m, std::regex{"^.*(?=:)"})) // Caso exista rótulo
        {
            linha.rotulo = m.str();           // Salva o rótulo
            trim(linha.rotulo);               // Guardar o rótulo sem espaços
            s.erase(0, m.str().length() + 1); // Apaga rótulo da string original
            trim(s);

            if (s == " " || s == "") // Caso o rótulo esteja sozinho na linha
            {
                linha.operacao = "";
                linha.operandos = "";
                linha.contador_linha = contador_linha;
                lista.push_back(linha);
                continue;
            }
        }
        else // Caso não exista rótulo
            linha.rotulo = "";

        tokens = std::move(tokenize(s, " ")); // Separa o resto da linha em tokens

        linha.operacao = tokens[0];
        linha.contador_linha = contador_linha;

        if (tokens.size() >= 2) // Se algum operando foi fornecido
            linha.operandos = tokens[1];
        else
            linha.operandos = ""; // Caso contrário

        if (tokens.size() > 2) // Mais de dois tokens significa que tem coisa separada por espaço
        {
            TrataErros::erroEspacosEntreOp(linha);
        }
        lista.push_back(linha); // Coloca a linha formada no final da lista
    }
    lista.splice(lista.end(), listaData); // Adiciona a sessão de dados depois da sessão de texto
    inputFile.close();                    // Fecha o arquivo de entrada

    /*
    Primeira passagem:
    Roda o programa todo para montar a tabela de símbolos
    */
    int contador_posicao = 0;

    std::map<std::string, int> tabelaSimbolos{};

    for (auto l : lista)
    {
        if (l.rotulo != "")
        {
            if (!validIdentifier(l.rotulo))
                TrataErros::erro6(l); // Indetificador inválido
            if (tabelaSimbolos.count(l.rotulo) > 0) // Se o rótulo já existe.
                TrataErros::erro2(l); // Símbolo redefinido
            else
                tabelaSimbolos[l.rotulo] = contador_posicao;
        }

        if (l.operacao == "") // Sem instrução
            continue;
        else if (tabelaInstrucoes.count(l.operacao) > 0)                   // Com instrução
            contador_posicao += std::get<1>(tabelaInstrucoes[l.operacao]); // Soma o tamanho da instrução
        else // Diretivas
        {
            if (l.operacao == "space")
                contador_posicao += 1;
            else if (l.operacao == "const")
                contador_posicao += 1;
            else
                TrataErros::erro3(l); // Diretiva ou inst inválida
        }
    }

    /*
    Segunda passagem
    Monta o arquivo .obj, além de checar os outros tipos de erros.
    */
    contador_posicao = 0;
    for (auto l : lista)
    {
        tokens = std::move(tokenize(l.operandos, ","));

        if (l.operacao == "")
            continue;
        else if (tabelaInstrucoes.count(l.operacao) > 0) // Se for uma instrução
        {
            contador_posicao += std::get<1>(tabelaInstrucoes[l.operacao]); // Soma o tamanho da instrução

            if (std::get<1>(tabelaInstrucoes[l.operacao]) - 1 != tokens.size()) // Número de tokens for diferente do esperado
                TrataErros::erro4(l); // Qtd de op inválida
            else
            {
                outputFile << std::get<0>(tabelaInstrucoes.at(l.operacao)) << " "; // Escreve o opcode
                for (auto token : tokens)
                {
                    if (!validIdentifier(token))
                        TrataErros::erro5(token, l); // Tipo de op inválido
                    else if (tabelaSimbolos.count(token) == 0)
                        TrataErros::erro1(token, l); // Símbolo indefinido
                    outputFile << tabelaSimbolos[token] << " "; // Escreve o operando
                }
            }
        }
        else // Se for uma diretiva
        {
            if (l.operacao == "space")
            {
                contador_posicao += 1;
                outputFile << "0"
                           << " ";

                if (tokens.size() != 0)
                    TrataErros::erro4(l); // Quantidade de op inválida
            }
            else if (l.operacao == "const")
            {
                contador_posicao += 1;

                if (tokens.size() != 1)
                    TrataErros::erro4(l); // Quantidade de op inválida
                else if (std::regex_match(tokens[0], std::regex{"^-?[0-9]+$"}))          // Se for um número inteiro positivo ou negativo
                    outputFile << tokens[0] << " ";                                      // Imprime o número na saída.
                else if (std::regex_match(tokens[0], std::regex{"^-?[0-9]+\\.[0-9]+$"})) // Números com casas decimais proibidos
                    TrataErros::erroNumeroFracionario(tokens[0], l);
                else // Não é um número
                    TrataErros::erro5(tokens[0], l); // Tipo de op inválido
            }
        }
    }
    outputFile.close();

    if (TrataErros::erroFlag)
        std::cout << "Falha ao montar\n";
    else
        std::cout << "Montado com sucesso.\n";

    return 0;
}
