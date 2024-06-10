#!/bin/bash

SISTEMAS=("vazio" "10mega" "50mega")
ARQUIVOS=("lorem-1M" "lorem-10M" "lorem-30M")
ESTADOS=("30niveis-60pornivel" "30niveis-vazio")


for sistema in ${SISTEMAS[@]}; do
    cp ./sistemas/$sistema ./
    for arquivo in ${ARQUIVOS[@]}; do
        echo "Executando teste de cópia e remoção com $sistema e $arquivo" >> ./resultados.txt
        soma_add=0
        soma_rem=0
        for i in {1..30}; do
            start="$(date +'%s.%N')"
            echo -e "monta $sistema\ncopia $arquivo /$arquivo\ndesmonta\nsai" | ./ep3
            tempo=$(echo "$(date +"%s.%N") - ${start}" | bc)
            soma_add=$(echo "$soma_add + $tempo" | bc)

            start="$(date +'%s.%N')"
            echo -e "monta $sistema\napaga /$arquivo\ndesmonta\nsai" | ./ep3 
            tempo=$(echo "$(date +"%s.%N") - ${start}" | bc)
            soma_rem=$(echo "$soma_rem + $tempo" | bc)
        done
        media_add=$(echo "scale=6; $soma_add / 30" | bc)
        media_rem=$(echo "scale=6; $soma_rem / 30" | bc)
        echo "Tempo médio para copiar: $media_add" >> ./resultados.txt
        echo "Tempo médio para remover: $media_rem" >> ./resultados.txt
    done
    rm $sistema

    for estado in ${ESTADOS[@]}; do
        echo "Executando teste de cópia e remoção de hierarquia vazio e cheio $sistema e $estado" >> ./resultados.txt
        soma=0
        for i in {1..30}; do
            cp ./sistemas/$sistema ./
            ./ep3 < "./estados/$estado$sistema" 
            start="$(date +'%s.%N')"
            echo -e "monta $sistema\napagadir /!\ndesmonta\nsai" | ./ep3 
            tempo=$(echo "$(date +"%s.%N") - ${start}" | bc)
            soma=$(echo "$soma + $tempo" | bc)
            rm $sistema
        done
        media=$(echo "scale=6; $soma / 30" | bc)
        echo "Tempo médio para 30 niveis: $media" >> ./resultados.txt
        echo
    done
done