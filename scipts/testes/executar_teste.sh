#!/bin/bash

ARQUIVO=$1
DCLIENT="../bin/dclient"

tempo_add=0
tempo_consultar=0
tempo_remover=0
tempo_count=0
tempo_search=0
count_add=0
count_consultar=0
count_remover=0
count_count=0
count_search=0

total_start=$(date +%s.%N)

while IFS= read -r linha || [ -n "$linha" ]; do
    tipo=$(echo "$linha" | awk '{print $1}')

    START=$(date +%s.%N)

    case "$tipo" in
        -a)
            eval $DCLIENT $linha > /dev/null 2>&1
            END=$(date +%s.%N)
            tempo_add=$(echo "$tempo_add + ($END - $START)" | bc)
            ((count_add++))
            ;;
        -c)
            echo "CMD: $DCLIENT $linha"
            eval $DCLIENT $linha
            END=$(date +%s.%N)
            tempo_consultar=$(echo "$tempo_consultar + ($END - $START)" | bc)
            ((count_consultar++))
            ;;
        -d)
            eval $DCLIENT $linha > /dev/null 2>&1
            END=$(date +%s.%N)
            tempo_remover=$(echo "$tempo_remover + ($END - $START)" | bc)
            ((count_remover++))
            ;;
        -l)
            eval $DCLIENT $linha > /dev/null 2>&1
            END=$(date +%s.%N)
            tempo_count=$(echo "$tempo_count + ($END - $START)" | bc)
            ((count_count++))
            ;;
        -s)
            eval $DCLIENT $linha > /dev/null 2>&1
            END=$(date +%s.%N)
            tempo_search=$(echo "$tempo_search + ($END - $START)" | bc)
            ((count_search++))
            ;;
    esac
done < "$ARQUIVO"

total_end=$(date +%s.%N)
total_tempo=$(echo "$total_end - $total_start" | bc)

echo ""
echo "tempo total - $total_tempo segundos"
if ((count_add > 0)); then
    echo "tempo médio a adicionar - $(echo "$tempo_add / $count_add" | bc -l)"
fi
if ((count_remover > 0)); then
    echo "tempo médio a remover - $(echo "$tempo_remover / $count_remover" | bc -l)"
fi
if ((count_consultar > 0)); then
    echo "tempo médio a consultar - $(echo "$tempo_consultar / $count_consultar" | bc -l)"
fi
if ((count_count > 0)); then
    echo "tempo médio a keyword Linha - $(echo "$tempo_count / $count_count" | bc -l)"
fi
if ((count_search > 0)); then
    echo "tempo médio a keyword todos ficheiros - $(echo "$tempo_search / $count_search" | bc -l)"
fi
