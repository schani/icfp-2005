(* $Id: timeout.ml,v 1.1 2005/07/05 10:58:09 heinz Exp $
 *
 * timeout - something produced during ICFP Contest 2005
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

(* the timeout API
 *
 * there is a simple API do do things like in parser_test.c
 *
 * val simple_timer_reset : unit -> unit
 *   will reset the simple timer to zero
 *   will be called at module load time
 *
 * val simple_timer_query : unit -> float
 *   will return the time since last reset
 *   can be used this way: if (simple_timer_query ()) > 4.0 then Unparser.do ()
 *
 * this is a complexer API that tries to be very cool, but it may be very bad
 * it lets you install a closure to be called after a timeout
 * the closure can be changed while the counter is counting, but you can
 * not be sure what was actually called .. is this bad?
 *
 * val complex_timer_install : (unit -> unit) -> float -> unit
 *   will install a complex timer
 *   when the time passed the closure will be executed
 *
 * val complex_timer_replace : (unit -> unit) -> unit
 *   replaces the closure without touching the timer
 *
 * val complex_timer_cancel : unit -> unit
 *   will cancel a comple timer
 *   once the closure starts to execute a counter will be increased, so
 *   you can check if it was triggered before it was canceld
 *
 * val complex_timer_get_counter : unit -> int
 *   returns how often a complex timer was actually executed
 *   this will not increase if a timer was canceled
 *)

exception Timeout_internal_fatal_error

let simple_timer = ref 0.0

let simple_timer_reset () =
  simple_timer := Unix.gettimeofday ()

let _ =
  simple_timer_reset ()

let simple_timer_query () =
  (Unix.gettimeofday ()) -. !simple_timer

let complex_timer_counter = ref 0

let complex_timer_get_counter () =
  !complex_timer_counter

let complex_timer_fun = ref (fun () -> ())

let complex_timer_replace f =
  complex_timer_fun := f

let complex_timer_install_nop =
  complex_timer_replace (fun () -> ())

let complex_timer_install f t =
  ignore (Unix.setitimer Unix.ITIMER_REAL { Unix.it_interval = 0.0;
					    Unix.it_value = t });
  complex_timer_replace f

let complex_timer_cancel () =
  complex_timer_install_nop;
  ignore (Unix.setitimer Unix.ITIMER_REAL { Unix.it_value = 0.0;
					    Unix.it_interval = 0.0; })

let complex_timer_sighandler i =
  if i != Sys.sigalrm then
    raise Timeout_internal_fatal_error;
  complex_timer_counter := !complex_timer_counter + 1;
  complex_timer_cancel (); (* redundant, but sure is sure *)
  !complex_timer_fun ()

let _ =
  Sys.set_signal Sys.sigalrm (Sys.Signal_handle complex_timer_sighandler)
