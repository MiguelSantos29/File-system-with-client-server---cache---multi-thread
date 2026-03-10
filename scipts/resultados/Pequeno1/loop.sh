#!/bin/bash

# Força o locale C para usar ponto como separador decimal
export LC_ALL=C

if [ $# -ne 1 ]; then
    echo "Uso: $0 <número_de_iterações>"
    exit 1
fi

soma_total=0
n=$1

tempos=()
iteracoes=()
outputs=()
tamanhos_indice=()

for ((i=1; i<=n; i++))
do
    echo "Iteração $i"
    make -C .. cleanData
    output=$(./executar_teste.sh teste1.txt)

    tempo_total=$(echo "$output" | grep "tempo total" | awk '{print $4}')
    if [ -z "$tempo_total" ]; then
        echo "Erro: Não foi possível extrair o tempo total na iteração $i"
        continue
    fi

    tempos+=("$tempo_total")
    iteracoes+=("$i")
    outputs[$i]="$output"

    # Obter o tamanho da pasta ../indice
    tamanho_indice=$(du -sk ../indice 2>/dev/null | awk '{print $1}')
    echo "Tamanho da pasta ../indice na iteração $i: $tamanho_indice KB"

    # Verifica se o tamanho extraído é um número válido
    if [[ "$tamanho_indice" =~ ^[0-9]+$ ]]; then
        tamanhos_indice+=("$tamanho_indice")
    else
        tamanhos_indice+=(0)
    fi

    echo "$output"
done

if [ ${#tempos[@]} -eq 0 ]; then
    echo "Erro: Nenhum tempo total válido foi coletado."
    exit 1
fi

# Ordenação e obtenção dos 3 melhores tempos
temp_file=$(mktemp)
for i in "${!tempos[@]}"; do
    echo "${tempos[$i]} ${iteracoes[$i]}" >> "$temp_file"
done
melhores=($(sort -n "$temp_file" | head -n 3))
rm -f "$temp_file"

echo -e "\nOs 3 melhores tempos totais:"
for i in {0..2}; do
    tempo="${melhores[$((i*2))]}"
    iteracao="${melhores[$((i*2+1))]}"
    if [ -n "$tempo" ] && [ -n "$iteracao" ]; then
        echo "$tempo iteração $iteracao"
        soma_total=$(echo "$soma_total + $tempo" | bc)
    fi
done

# Somas das métricas para as melhores iterações
soma_adicionar=0
soma_remover=0
soma_consultar=0
soma_keyword_linha=0
soma_keyword_todos=0
contador=0

for i in {0..2}; do
    tempo="${melhores[$((i*2))]}"
    iteracao="${melhores[$((i*2+1))]}"
    if [ -z "$iteracao" ]; then
        continue
    fi
    output="${outputs[$iteracao]}"
    adicionar=$(echo "$output" | grep "tempo médio a adicionar" | sed 's/.*- //')
    remover=$(echo "$output" | grep "tempo médio a remover" | sed 's/.*- //')
    consultar=$(echo "$output" | grep "tempo médio a consultar" | sed 's/.*- //')
    keyword_linha=$(echo "$output" | grep "tempo médio a keyword Linha" | sed 's/.*- //')
    keyword_todos=$(echo "$output" | grep "tempo médio a keyword todos ficheiros" | sed 's/.*- //')

    echo "Iteração $iteracao métricas extraídas:"
    echo "adicionar=$adicionar"
    echo "remover=$remover"
    echo "consultar=$consultar"
    echo "keyword_linha=$keyword_linha"
    echo "keyword_todos=$keyword_todos"

    # Verifica se todas as métricas são números válidos
    if ! [[ "$adicionar" =~ ^[0-9]*\.[0-9]+$ ]] || \
       ! [[ "$remover" =~ ^[0-9]*\.[0-9]+$ ]] || \
       ! [[ "$consultar" =~ ^[0-9]*\.[0-9]+$ ]] || \
       ! [[ "$keyword_linha" =~ ^[0-9]*\.[0-9]+$ ]] || \
       ! [[ "$keyword_todos" =~ ^[0-9]*\.[0-9]+$ ]]; then
        echo "Aviso: Alguma métrica inválida na iteração $iteracao."
        continue
    fi

    soma_adicionar=$(echo "$soma_adicionar + $adicionar" | bc)
    soma_remover=$(echo "$soma_remover + $remover" | bc)
    soma_consultar=$(echo "$soma_consultar + $consultar" | bc)
    soma_keyword_linha=$(echo "$soma_keyword_linha + $keyword_linha" | bc)
    soma_keyword_todos=$(echo "$soma_keyword_todos + $keyword_todos" | bc)
    contador=$((contador + 1))
done

if [ $contador -eq 0 ]; then
    echo "Erro: Nenhuma iteração válida para calcular as médias."
    exit 1
fi

media_adicionar=$(echo "scale=6; $soma_adicionar / $contador" | bc)
media_remover=$(echo "scale=6; $soma_remover / $contador" | bc)
media_consultar=$(echo "scale=6; $soma_consultar / $contador" | bc)
media_keyword_linha=$(echo "scale=6; $soma_keyword_linha / $contador" | bc)
media_keyword_todos=$(echo "scale=6; $soma_keyword_todos / $contador" | bc)
media_tempo_total=$(echo "scale=6; $soma_total / $contador" | bc)

# Encontrar o maior tamanho da pasta ../indice
max_tamanho=0
for t in "${tamanhos_indice[@]}"; do
    if [[ "$t" =~ ^[0-9]+$ ]] && [ "$t" -gt "$max_tamanho" ]; then
        max_tamanho=$t
    fi
done

echo -e "\nMédias das métricas para as 3 melhores iterações:"
echo "Média tempo total: $media_tempo_total"
echo "Média a adicionar: $media_adicionar"
echo "Média a remover: $media_remover"
echo "Média a consultar: $media_consultar"
echo "Média a keyword Linha: $media_keyword_linha"
echo "Média a keyword todos ficheiros: $media_keyword_todos"
echo "Tamanho máximo da pasta ../indice: ${max_tamanho} KB"
