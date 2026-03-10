#!/bin/bash

OUTPUT_FILE="pedidostotal.txt"
WORDS=("book" "love" "sky" "tree" "house" "river" "star" "moon" "sun" "bird")
RECENT_KEYS=()

# Function to add a key to recent keys (for LRU testing)
add_recent_key() {
    RECENT_KEYS+=("$1")
    # Keep only the last 20 recent keys to avoid growing too large
    if [ ${#RECENT_KEYS[@]} -gt 20 ]; then
        RECENT_KEYS=("${RECENT_KEYS[@]:1}")
    fi
}

# Function to get a random recent key (or last_key - 15 if none available)
get_key() {
    local last_key="$1"
    if [ ${#RECENT_KEYS[@]} -gt 0 ]; then
        # 80% chance to pick a recent key, 20% chance to use last_key - 15
        if [ $((RANDOM % 100)) -lt 80 ]; then
            local index=$((RANDOM % ${#RECENT_KEYS[@]}))
            echo "${RECENT_KEYS[$index]}"
            return
        fi
    fi
    # Calculate last_key - 15
    local last_num=$(echo "$last_key" | sed 's/\.txt$//')
    local key_num=$((last_num - 15))
    if [ $key_num -ge 1001 ]; then
        echo "$key_num.txt"
    else
        # Fallback to a recent key or the oldest available
        if [ ${#RECENT_KEYS[@]} -gt 0 ]; then
            echo "${RECENT_KEYS[0]}"
        else
            echo "1001.txt"
        fi
    fi
}

# Write first 15 -a entries from PedidosAdicionar.txt
cat > "$OUTPUT_FILE" << EOL
-a "The Wallet of Kai Lung" "Ernest Bramah" "1997" "1076.txt"
-a "Far from the Madding Crowd" "Thomas Hardy" "1994" "107.txt"
-a "Sword Blades and Poppy Seed" "Amy Lowell" "1997" "1020.txt"
-a "The Life and Opinions of Tristram Shandy, Gentleman" "Laurence Sterne" "1997" "1079.txt"
-a "The Scouts of the Valley" "Joseph A. Altsheler" "1997" "1078.txt"
-a "The Wrecker" "Robert Louis Stevenson,Lloyd Osbourne" "2006" "1024.txt"
-a "The Arrow of Gold" "Joseph Conrad" "2009" "1083.txt"
-a "A Modest Proposal" "Jonathan Swift" "1997" "1080.txt"
-a "Around the World in 80 Days" "Jules Verne" "2008" "103.txt"
-a "Rolf In The Woods" "Ernest Thompson Seton" "1997" "1088.txt"
-a "Baartock" "Lewis Roth" "1997" "1087.txt"
-a "Poems" "Wilfred Owen" "1997" "1034.txt"
-a "The Bickerstaff-Partridge Papers" "Jonathan Swift" "1997" "1090.txt"
-a "Renascence and Other Poems" "Edna St. Vincent Millay" "2008" "109.txt"
-a "The Life of John Bunyan" "Edmund Venables" "2005" "1037.txt"
EOL

# Initialize key tracking
last_key="1037.txt"
add_recent_key "$last_key"
for key in 107 1020 1079 1078 1024 1083 1080 103 1088 1087 1034 1090 109 1076; do
    add_recent_key "$key.txt"
done

# Operation counts for remaining 1000 lines
COUNT_A=150
COUNT_C=300
COUNT_L=300
COUNT_S=200
COUNT_D=50

# Generate operation array
OPERATIONS=()
for ((i=0; i<COUNT_A; i++)); do OPERATIONS+=("a"); done
for ((i=0; i<COUNT_C; i++)); do OPERATIONS+=("c"); done
for ((i=0; i<COUNT_L; i++)); do OPERATIONS+=("l"); done
for ((i=0; i<COUNT_S; i++)); do OPERATIONS+=("s"); done
for ((i=0; i<COUNT_D; i++)); do OPERATIONS+=("d"); done

# Shuffle operations (using Fisher-Yates shuffle)
for ((i=${#OPERATIONS[@]}-1; i>0; i--)); do
    j=$((RANDOM % (i + 1)))
    tmp="${OPERATIONS[$i]}"
    OPERATIONS[$i]="${OPERATIONS[$j]}"
    OPERATIONS[$j]="$tmp"
done

# Generate remaining 1000 lines
for op in "${OPERATIONS[@]}"; do
    case "$op" in
        a)
            # Increment key
            last_num=$(echo "$last_key" | sed 's/\.txt$//')
            last_num=$((last_num + 1))
            last_key="$last_num.txt"
            # Use a generic -a entry
            echo "-a \"Book $last_num\" \"Author $last_num\" \"2000\" \"$last_key\"" >> "$OUTPUT_FILE"
            add_recent_key "$last_key"
            ;;
        c)
            key=$(get_key "$last_key")
            echo "-c \"$key\"" >> "$OUTPUT_FILE"
            add_recent_key "$key"
            ;;
        l)
            key=$(get_key "$last_key")
            word=${WORDS[$((RANDOM % ${#WORDS[@]}))]}
            echo "-l \"$key\" \"$word\"" >> "$OUTPUT_FILE"
            add_recent_key "$key"
            ;;
        s)
            word=${WORDS[$((RANDOM % ${#WORDS[@]}))]}
            echo "-s \"$word\" 4" >> "$OUTPUT_FILE"
            ;;
        d)
            key=$(get_key "$last_key")
            echo "-d \"$key\"" >> "$OUTPUT_FILE"
            # Remove key from recent keys (optional, depending on LRU behavior)
            TMP_KEYS=()
            for k in "${RECENT_KEYS[@]}"; do
                if [ "$k" != "$key" ]; then
                    TMP_KEYS+=("$k")
                fi
            done
            RECENT_KEYS=("${TMP_KEYS[@]}")
            ;;
    esac
done

echo "Generated $OUTPUT_FILE with 1015 lines."