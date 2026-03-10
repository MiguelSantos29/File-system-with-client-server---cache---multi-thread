#!/bin/bash

ARQUIVO=$1
DCLIENT="../bin/dclient"

# Global variables for accumulating times (in nanoseconds) and counters
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

total_start=$(gdate +%s.%N)

while IFS= read -r linha || [ -n "$linha" ]; do
    tipo=$(echo "$linha" | awk '{print $1}')
    START=$(gdate +%s%N)  # Time in nanoseconds (integer)

    case "$tipo" in
        -a)
            eval $DCLIENT $linha > /dev/null 2>&1
            END=$(gdate +%s%N)
            tempo_add=$((tempo_add + (END - START)))
            ((count_add++))
            ;;
        -c)
            eval $DCLIENT $linha > /dev/null 2>&1
            END=$(gdate +%s%N)
            tempo_consultar=$((tempo_consultar + (END - START)))
            ((count_consultar++))
            ;;
        -d)
            eval $DCLIENT $linha > /dev/null 2>&1
            END=$(gdate +%s%N)
            tempo_remover=$((tempo_remover + (END - START)))
            ((count_remover++))
            ;;
        -l)
            eval $DCLIENT $linha > /dev/null 2>&1
            END=$(gdate +%s%N)
            tempo_count=$((tempo_count + (END - START)))
            ((count_count++))
            ;;
        -s)
            eval $DCLIENT $linha > /dev/null 2>&1
            END=$(gdate +%s%N)
            tempo_search=$((tempo_search + (END - START)))
            ((count_search++))
            ;;
    esac
done < "$ARQUIVO"

eval $DCLIENT -f
total_end=$(gdate +%s.%N)
total_tempo=$(echo "$total_end - $total_start" | bc)

# Display results, converting times from nanoseconds to seconds
echo ""
echo "tempo total - $total_tempo segundos"
if ((count_add > 0)); then
    echo "tempo médio a adicionar - $(echo "scale=6; $tempo_add / $count_add / 1000000000" | bc)"
fi
if ((count_remover > 0)); then
    echo "tempo médio a remover - $(echo "scale=6; $tempo_remover / $count_remover / 1000000000" | bc)"
fi
if ((count_consultar > 0)); then
    echo "tempo médio a consultar - $(echo "scale=6; $tempo_consultar / $count_consultar / 1000000000" | bc)"
fi
if ((count_count > 0)); then
    echo "tempo médio a keyword Linha - $(echo "scale=6; $tempo_count / $count_count / 1000000000" | bc)"
fi
if ((count_search > 0)); then
    echo "tempo médio a keyword todos ficheiros - $(echo "scale=6; $tempo_search / $count_search / 1000000000" | bc)"
fi