# Output every line except ones that contains match_string. Also, do not output
# extra_lines_to_skip lines that follow the line that contained match_string.
function skip() {
  if [ $# -ne 3 ] && [ $# -ne 2 ]; then
    echo "Usage: $0 match_string extra_lines_to_skip [input_file]"
    return 1
  fi
  local -r match_string="$1"
  local -r lines_to_skip="$2"

  if [ $# -eq 2 ]; then
    awk "/$match_string/{skip=$lines_to_skip;next} skip>0{--skip;next} {print}"
  else
    awk "/$match_string/{skip=$lines_to_skip;next} skip>0{--skip;next} {print}" "$3"
  fi
  return 0
}

# Modify input_file to remove lines that contains match_string. Also remove
# extra_lines_to_skip lines that follow the line that contained match_string.
function skip_inplace() {
  if [ $# -ne 3 ]; then
    echo "Usage: $0 match_string lines_to_skip input_file"
    return 1
  fi
  local -r match_string="$1"
  local -r lines_to_skip="$2"
  local -r input_file="$3"

  local -r temp_file=$(mktemp)
  skip "$match_string" "$lines_to_skip" "$input_file" > "$temp_file"
  mv "$temp_file" "$input_file"
}
