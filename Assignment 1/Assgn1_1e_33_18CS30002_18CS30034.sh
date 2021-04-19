len="${1:-16}"
(("$len"<0))&&exit;
tr -dc 'A-Za-z0-9_'</dev/urandom|head -c $len;echo
