(* $Id: unparser.ml,v 1.4 2005/07/05 19:48:18 signof Exp $
 *
 * unparser - something produced during ICFP Contest 2005
 * Copyright (C) 2005 Martin Biely
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

open World

exception Unparse_error of string

let unparse_error exp got =
  raise (Unparse_error
	   (Printf.sprintf
	      "%s: unparse error: expecting %s but got '%s'"
	      Sys.argv.(0) exp got))

type token_sep = 
  | TS_Space
  | TS_Newline
  | TS_None
  | TS_Other of string

let string_of_bot b:bot = b       

let string_of_ptype = function
  | PTYPE_Cop_Foot -> "cop-foot"
  | PTYPE_Cop_Car  -> "cop-car"
  | PTYPE_Robber -> "robber"

let os = stdout

let output_sep = function
  | TS_Space -> output_string os " "
  | TS_Newline -> output_string os "\n"
  | TS_Other(s) -> output_string os s
  | TS_None -> ()
      

let output_endmessage () = 
  flush os

let output_token ?(sep=TS_Space) token = 
  output_string os token; output_sep sep

let rec output_token_list ?(sep=TS_Space) ?(endsep=TS_Newline) = function
  | [] -> unparse_error "<non empty list>" "[]"
  | [t] -> output_token ~sep:endsep t
  | t::ts -> (
      output_token ~sep:sep t;
      output_token_list ~sep:sep ~endsep:endsep ts
    )

let rec output_token_list_of_lists ?(sep=TS_Space) ?(listsep=TS_Newline) ?(endsep=TS_Newline) = function
  | [] -> unparse_error "<non empty list of lists>" "[]"
  | [l] -> output_token_list ~sep:sep ~endsep:endsep l
  | l::ls -> (
      output_token_list ~sep:sep ~endsep:listsep l; 
      output_token_list_of_lists ~sep:sep ~listsep:listsep ~endsep:endsep ls
    )
    
let unparse_register (name,ptype) = 
  output_token_list ["reg:"; name; (string_of_ptype ptype)];
  output_endmessage ()


let unparse_move (node, ptype) = 
  output_token_list ["mov:"; node.node_loc; (string_of_ptype ptype)];
  output_endmessage ()
    
let unparse_informs inform_list = 
  let list_of_inform i = 
    ["inf:"; (string_of_bot i.inf_player); i.inf_node.node_loc; 
     (string_of_ptype i.inf_ptype); (string_of_int i.inf_world); 
     (string_of_int i.inf_certainty)]
  in
  let rec loop acc = function
    | [] -> acc
    | i::is -> loop ((list_of_inform i)::acc) is
  in
  output_token_list_of_lists (["inf\\"]::(loop [["inf/"]] inform_list));
  output_endmessage ()

let unparse_plan plan =
  let list_of_plan i =
    ["plan:"; (string_of_bot i.plan_player); i.plan_node.node_loc; 
     (string_of_ptype i.plan_ptype); (string_of_int i.plan_world)]
  in
  let rec loop acc = function
    | [] -> acc
    | i::is -> loop ((list_of_plan i)::acc) is
  in
  output_token_list_of_lists (["plan\\"]::(loop [["plan/"]] plan));
  output_endmessage ()

let unparse_voting votes = 
  let list_of_vote v = 
    ["vote:"; (string_of_bot v)]
  in 
  let rec loop last = function 
      (* not tail recursive. we don't want to reverse the vote ...*)
    | [] -> last
    | v::vs -> (list_of_vote v)::(loop last vs)
  in
  output_token_list_of_lists (["vote\\"]::(loop [["vote/"]] votes));
  output_endmessage ()
  
    
