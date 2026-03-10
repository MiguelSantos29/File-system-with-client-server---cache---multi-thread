#!/bin/bash
mkdir -p testes
OUTFILE="testes/test_random.txt"
rm -f "$OUTFILE"

total=100000
add=$((total * 5 / 100))
consult=$((total * 30 / 100))
remove=$((total * 5 / 100))
count=$((total * 30 / 100))
search=$((total * 0 / 100))

for ((i=1; i<=consult; i++)); do
    id=$(( (RANDOM % 50000) + 1 ))
    echo "-c $id" >> "$OUTFILE"
done

for ((i=1; i<=remove; i++)); do
    id=$(( (RANDOM % 50000) + 1 ))
    echo "-d $id" >> "$OUTFILE"
done

for ((i=1; i<=count; i++)); do
    id=$(( (RANDOM % 50000) + 1 ))
    palavra=$(shuf -e are to be make perfect -n 1)
    echo "-l $id $palavra" >> "$OUTFILE"
done

for ((i=1; i<=search; i++)); do
    palavra=$(shuf -e are to be make perfect -n 1)
    echo "-s $palavra 16" >> "$OUTFILE"
done

for ((i=1; i<=add; i++)); do
    echo "-a \"Titulo$i\" \"Autor$i\" \"2024\" \"doc$i.txt\"" >> "$OUTFILE"
done

shuf --random-source=<(yes 42) "$OUTFILE" -o "$OUTFILE"
echo "Teste RANDOM gerado: $OUTFILE"
