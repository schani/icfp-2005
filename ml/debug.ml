
let do_debug = true
let max_lvl = 10

let debug_string lvl str = 
  if do_debug && lvl < max_lvl then (
    Printf.fprintf stderr "%s: %s\n" Sys.argv.(0) str;
    flush stderr
  )

