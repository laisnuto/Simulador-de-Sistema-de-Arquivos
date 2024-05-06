#!/bin/bash

PEQUENA=625
MEDIA=1250
GRANDE=2500

POUCOS=125
NORMAL=250
MUITOS=500

EXPERIMENTOS_PISTA=($PEQUENA $MEDIA $GRANDE)

make

MEMORIA_FINAL=()
TEMPO_FINAL=()
teste
for i in ${EXPERIMENTOS_PISTA[@]}
do
    EXPERIMENTOS_CICLISTAS=($POUCOS $NORMAL $MUITOS)

    for j in ${EXPERIMENTOS_CICLISTAS[@]}
    do
        echo "$i $j:"

        MEMORIA=()
        TEMPO=()

        #for k in {1..30}
        #do 
            MEMORIA_TEMP=()
            ./ep2 $i $j &
            PID=$!
            ini=$(date +%s%N)
            while kill -0 $PID 2> /dev/null; do
                #MEMORIA_TEMP+=($(top -bn 1 | grep ep2-sem-print | awk '{print $6}'))
                MEMORIA_TEMP+=($(grep VmRSS /proc/$PID/status | awk '{print $2}'))
                sleep 0.1
            done
            fim=$(date +%s%N)
            duracao=$(($fim - $ini))

            TEMPO+=($duracao)
            MEMORIA+=($(echo "${MEMORIA_TEMP[@]}" | awk '{ sum += $1 } END { print sum / NR }'))
        #done

          echo "Resultado do experimento com tamanho de pista: $i e número de ciclistas: $j"
        echo -e "Uso da memória: $(echo "${MEMORIA[@]}" | awk '{ sum += $1 } END { print sum / NR }')"
        echo -e "Tempo (ns): $(echo "${TEMPO[@]}" | awk '{ sum += $1 } END { print sum / NR / 1000000000 }') \n"


        MEMORIA_FINAL+=($(echo "${MEMORIA[@]}" | awk '{ sum += $1 } END { print sum / NR }'))
        TEMPO_FINAL+=($(echo "${TEMPO[@]}" | awk '{ sum += $1 } END { print sum / NR / 1000000000 }'))




    done
done

echo "PEQUENA: POUCOS NORMAL MUITOS"
echo "USO DA MEMORIA: ${MEMORIA_FINAL[@]:0:3}"
echo "TEMPO (ns): ${TEMPO_FINAL[@]:0:3}"

echo "MEDIA: POUCOS NORMAL MUITOS"
echo "USO DA MEMORIA: ${MEMORIA_FINAL[@]:3:3}"
echo "TEMPO (ns): ${TEMPO_FINAL[@]:3:3}"

echo "GRANDE: POUCOS NORMAL MUITOS"
echo "USO DA MEMORIA: ${MEMORIA_FINAL[@]:6:3}"
echo "TEMPO (ns): ${TEMPO_FINAL[@]:6:3}"