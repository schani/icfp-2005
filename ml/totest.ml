(* $Id: totest.ml,v 1.1 2005/07/05 10:58:09 heinz Exp $
 *
 * totest - test for timeout module
 * Copyright (C) 2005  Heinz Deinhart
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *)

open Timeout

let msg str =
  print_string str;
  flush stdout
  
let _ =
  let rec do_simple () =
    msg "  PRESS ENTER to continue  ['q' to abort]";
    if "q" = (read_line ()) then () else (
      msg "  calling sime_timer_query: ";
      msg (string_of_float (simple_timer_query ()));
      msg " @ "; ignore (Sys.command "date");
      do_simple ()
    )
  in
    msg "simple timer test:\n";
    msg "  resetting time";
    msg " @ "; ignore (Sys.command "date");
    msg "  calling sime_timer_query: ";
    msg (string_of_float (simple_timer_query ()));
    msg " @ "; ignore (Sys.command "date");
    do_simple ();
    msg "\n";
    msg "complex timer test:\n";
    let flag = ref 0
    in
      msg "  installing complex timer in 5 seconds";
      msg " @ "; ignore (Sys.command "date");
      complex_timer_install (fun () -> msg " -> TIMER OCCURED\n"; flag := 1)
	5.2; (* if you play with this value u can see few or more .s *)
      while !flag = 0 do
	(
	  try
	    ignore (Unix.select [] [] [] 0.1);
	  with _ -> ();
	);
	msg "."
      done;
      msg "  mainloop detected flag";
      msg " @ "; ignore (Sys.command "date");
      print_string "\ndone it\n"
