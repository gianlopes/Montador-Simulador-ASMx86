Trabalho 1 Software Básico
Gianlucas Dos Santos Lopes - 18/0041991

O trabalho está dividido entre os seguintes arquivos:
    montador.xpp: implementação geral das passagens do montador
    trataErros.xpp: uma classe para tratar do output dos erros encontrados no montador
    stringUtils.xpp: funções para lidar com strings (trim, toLower, etc...)
    simulador.cpp: implementação geral do simulador


O compilador usado foi o g++ versão 9, em um sistema linux (ubuntu)
Para compilar os dois executáveis, um makefile foi incluído, use:
    make


Para executar o montador use:
    ./montador aquivo.asm

(Será gerado "arquivo.obj")


Para executar o simulador use:
    ./simulador arquivo.obj

(Será gerado "arquivo.txt")

*As extensões são importantes*