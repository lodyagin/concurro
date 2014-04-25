BEGIN { FS="\t" }

/timeout$/ {
  a = $0
  $NF = ""
  wait[$0] = a
}

/signalled$/ {
  $NF = ""
  delete wait[$0]
}

END {
  for (w in wait)
    print wait[w]
}
