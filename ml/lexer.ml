(* $Id: lexer.ml,v 1.4 2005/07/01 20:07:55 heinz Exp $
 *
 * lexer - something produced during ICFP Contest 2005
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

exception Lexer_error

let line_nr = ref 0
let line = ref ""

let whitespace_regexp = Str.regexp "[ \t\n]+"
let pseudo_lexer_divide_line channel =
  try
    let new_line = input_line channel
    in
      line_nr := !line_nr + 1;
      line := new_line;
      Str.split whitespace_regexp new_line
  with
      _ -> raise Lexer_error

let rec pseudo_lexer ?(line_nr=0) channel =
  [< 'pseudo_lexer_divide_line channel;
     pseudo_lexer channel ~line_nr:(line_nr + 1)>]

let lexer = ref (pseudo_lexer stdin)

let next () =
  Stream.next !lexer

let get_linenr () =
  !line_nr
let get_line () =
  !line
