input="1.b.files"
output="1.b.files.out"
total="1.b.out.txt"
mkdir "$output"
touch "$total"
for files in "$input"/*;do
    sort -nr $files -o "$output"/"$(basename -- $files)"
done
sort -nmr "$output"/* -o "$total"
