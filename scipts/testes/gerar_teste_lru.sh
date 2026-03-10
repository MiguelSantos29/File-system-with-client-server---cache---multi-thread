#!/bin/bash
mkdir -p testes
OUTFILE="testes/test_lru.txt"
rm -f "$OUTFILE"

TSV_FILE="ola.tsv"   # <- Nome do ficheiro TSV que vai estar na mesma pasta que o script
                         #    (ajusta se for outro nome!)

total=115
add=$((total * 100/ 100))
consult=$((total * 0 / 100))
remove=$((total * 0 / 100))
count=$((total * 0 / 100))
search=$((total * 0 / 100))

# === GERAR ADIÇÕES PRIMEIRO ===
i=0
tail -n +2 "$TSV_FILE" | while IFS=$'\t' read -r filename title year authors; do
    if (( i < add )); then
        echo "-a \"$title\" \"$authors\" \"$year\" \"$filename\"" >> "$OUTFILE"
        ((i++))
    else
        break
    fi
done

# === GERAR CONSULTAS, REMOÇÕES, CONTAGENS e PESQUISAS ===
for ((i=1; i<=consult; i++)); do
    id=$(( (i%1000) - (i % 50)))
    echo "-c $id" >> "$OUTFILE"
done

for ((i=1; i<=remove; i++)); do
    id=$(( (i%1000) - (i % 50) ))
    echo "-d $id" >> "$OUTFILE"
done

for ((i=1; i<=count; i++)); do
    id=$(( (i%1000) - (i % 50) ))
    palavra=$(shuf -e are to be make perfect -n 1)
    echo "-l $id \"$palavra\"" >> "$OUTFILE"
done

for ((i=1; i<=search; i++)); do
    palavra=$(shuf -e are to be make perfect -n 1)
    echo "-s \"$palavra\" 4" >> "$OUTFILE"
done

# === MISTURAR TUDO ALEATORIAMENTE ===
shuf --random-source=<(yes 42) "$OUTFILE" -o "$OUTFILE"

echo "Teste LRU gerado: $OUTFILE"
