(* $Id: tsim.ml,v 1.5 2005/07/01 20:07:55 heinz Exp $
 *
 * tsim - something produced during ICFP Contest 2005
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

open World
open Worldmap
open Parser
open Lexer

let print_path wm src dst = 
  let _ = Printf.printf "FROM %s TO %s \n" src dst in
  begin
    iter_path wm (node_find src) (node_find dst) Move_Car
      (fun x -> 
	print_string (x.node_loc^" -> "));
    print_string "already there\n" 
  end

    
let _ =
  (*  let _ = (lexer :=  pseudo_lexer (open_in "parse.txt")) 
      in*) 
  let skel = parse_world_skeleton ()
  in let y = parse_world_message ()
  in let z = parse_inform_message ()
  in let z2 = parse_plan_message ()
  in let z3 = parse_vote_tally ()
  in let wm = build_worldmap skel 
  in let _ = print_path wm "54-and-blackstone" "54-and-kenwood"
  in let _ = print_path wm "54-and-kenwood" "54-and-blackstone"
  in
  print_string "done it\n"
