(* $Id: parser.ml,v 1.11 2005/07/09 13:48:23 heinz Exp $
 *
 * parser - something produced during ICFP Contest 2005
 * Copyright (C) 2005  Heinz Deinhart
 * Copyright (C) 2005  Martin Biely
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

(* the parser API:
 *
 * val parse_world_skeleton : unit -> world_skeleton
 *   will return a filled world_skeleton
 *   may raise Parse_error
 *
 * val parse_world_message : unit -> world_skeleton option
 *   will return Some world_message or None if the game is over
 *   may raise Parse_error
 *
 * val parse_vote_tally : unit -> string option
 *   will return Some winner or None if there is no winner
 *   may raise Parser_error
 *
 * val parse_inform_message : unit -> (string * inform list) list
 *   will return an assoc list containing source bot as key
 *    and its inform list as value
 *   may raise Parser_error
 *
 * val parse_plan_message : unit -> (string * plan list) list
 *   will return an assoc list containing source bot as key
 *    and its plan list as value
 *   may raise Parser_error
 *)

open World

exception Parse_error of string

let parse_error exp =
  raise (Parse_error
	   (Printf.sprintf
	      "%s: parse error in line %i: expecting %s but got '%s'"
	      Sys.argv.(0) (Lexer.get_linenr ()) exp (Lexer.get_line ())))

let ptype_of_string = function
  | "cop-foot" -> PTYPE_Cop_Foot 
  | "cop-car" -> PTYPE_Cop_Car 
  | "robber" -> PTYPE_Robber
  | x -> parse_error "<invalid ptype>"

let await_string name =
  match Lexer.next () with
      [ s ] when s = name -> ()
    | _ -> parse_error name

let receive_string name =
  match Lexer.next () with
      [ s1; s2 ] when s1 = name -> s2
    | _ -> parse_error name

let create_node name tag x y index =
  try
    let new_node = { node_loc = name;
		     node_x = float_of_string x;
		     node_y = float_of_string y;
		     node_index = index;
		     node_edges = [];
		     node_is_cul_de_sac = None;
		     node_foot_degree = None;
		     node_tag = match tag with
			 "hq" -> ND_HQ
		       | "bank" -> ND_Bank
		       | "robber-start" -> ND_RobberStart
		       | "ordinary" -> ND_Ordinary
		       | _ -> parse_error "hq|bank|robber-start|ordinary"
		   }
    in
      node_register new_node;
      new_node
  with
      _ ->
	parse_error "two float coordinates"

let create_edge src dst edgetype =
  let src = node_find src in
  let e = 
    { edge_from = src;
	edge_to = node_find dst;
	edge_type = match edgetype with
	    "car" -> ET_Car
	  | "foot" -> ET_Foot
	  | _ -> parse_error "car|foot"
    }
  in
    src.node_edges <- e::src.node_edges;
    e
    

let parse_world_skeleton () =
  let rec work ?(name="Unnamed") ?(robbers=[]) ?(cops=[]) ws =
    match Lexer.next () with
	[ "name:"; s ] -> work ~name:s ~robbers ~cops ws
      | [ "robber:"; s ] -> work ~name ~robbers:(s :: robbers) ~cops ws
      | [ "cop:"; s ] -> work ~name ~robbers ~cops:(s :: cops) ws
      | [ "nod\\" ] -> { ws with
			   ws_name = name;
			   ws_robbers = Array.of_list (List.rev robbers);
			   ws_cops =  Array.of_list (List.rev cops);
		       }
      | _ -> parse_error "(name|robber|cop):|nod\\"
  and read_nodes ws =
    match Lexer.next () with
	[ "nod:"; name; tag; x; y ] -> read_nodes
	  { ws with 
	    ws_nodes = (create_node name tag x y (ws.ws_num_nodes)) :: ws.ws_nodes;
	    ws_num_nodes = ws.ws_num_nodes + 1;
	  }
      | [ "nod/" ] -> ws
      | _ -> parse_error "nod[:/]"
  and read_edges ws =
    match Lexer.next () with
	[ "edg:"; src; dst; edgetype ] -> read_edges
	  { ws with ws_edges = (create_edge src dst edgetype) :: ws.ws_edges }
      | [ "edg/" ] -> ws
      | _ -> parse_error "edg[:/]"
  in
    try
      await_string "wsk\\";
      let ws = read_nodes (work { ws_name = "";
				  ws_robbers = [||]; ws_cops = [||];
				  ws_nodes = []; ws_edges = [];
				  ws_num_nodes = 0; })
      in
	await_string "edg\\";
	let ws2 = read_edges ws
	in
	  await_string "wsk/";
	  ws2
    with
	Parse_error x -> raise (Parse_error x)
      | _ -> raise (Parse_error "<something other in world skeleton>")

let node_int_pair = function
  | [name;value] -> (node_find name, int_of_string value)
  |  _ ->  parse_error "<location integer>"
       
let read_list_of id f = 
  let start = id^"\\" in
  let one = id^":" in
  let nomore = id^"/" in
  let rec read_something acc = 
    match Lexer.next () with
      | id :: rest when id = one -> read_something ((f rest)::acc)
      | [ id ] when id = nomore -> acc
      | _ -> parse_error (id^":|"^id^"/")
  in
    await_string start;
    read_something []

let player_location = function
  | [name; loc; ptype] -> (name, ((node_find loc), ptype_of_string ptype))
  | _ -> parse_error "<playername location ptype>"

let parse_world_message () =
  try
    match Lexer.next () with
	[ "game-over" ] -> None
      | [ "wor\\" ] ->
	  let world = int_of_string (receive_string "wor:")
	  in let loot = int_of_string (receive_string "rbd:")
	  in let bankvalue = read_list_of "bv" node_int_pair
	  in let evidence = read_list_of "ev" node_int_pair
	  in let smell = int_of_string (receive_string "smell:")
	  in let players = read_list_of "pl" player_location
	  in let wm = {
	      wm_world = world;
	      wm_loot = loot;
	      wm_bankvalue = bankvalue;
	      wm_evidence = evidence;
	      wm_smell = smell;
	      wm_players = players;
	    }
	  in
	    await_string "wor/";
	    Some wm
      | _ -> parse_error "game-over|wor\\"
  with
      Parse_error x -> raise (Parse_error x)
    | _ -> raise (Parse_error "<something other in world message>")

let parse_vote_tally () =
  try
    match Lexer.next () with
	[ "nowinner:" ] -> None
      | [ "winner:"; bot ] -> Some bot
      | _ -> parse_error "(no)?winner:"
  with
      Parse_error x -> raise (Parse_error x)
    | _ -> raise (Parse_error "<something other in vote tally>")

let rec parse_inform_from ?(inflist=[]) () =
  match Lexer.next () with
      [ "inf:"; bot; loc; ptype; world; certainty ] ->
	parse_inform_from
	  ~inflist:({ inf_player = bot; inf_node = (node_find loc);
		      inf_ptype = ptype_of_string ptype;
		      inf_world = int_of_string world;
		      inf_certainty = int_of_string certainty } :: inflist) ()
    | [ "inf/" ] -> inflist
    | _ -> parse_error "inf[:/]"

let rec parse_inform_message () =
  let rec work ?(result=[]) () =
    match Lexer.next () with
	[ "from:"; src ] ->
	  await_string "inf\\";
	  work ~result:((src, parse_inform_from ()) :: result) ()
      | [ "from/" ] ->
	  result
      | _ -> parse_error "from[:/]"
  in
    await_string "from\\";
    work ()

let rec parse_plan_from ?(planlist=[]) () =
  match Lexer.next () with
      [ "plan:";
	bot; loc; ptype; world; certainty ] ->
	parse_plan_from
	  ~planlist:({ plan_player = bot; plan_node = (node_find loc);
		       plan_ptype = ptype_of_string ptype;
		       plan_world = int_of_string world; } :: planlist) ()
    | [ "plan/" ] -> planlist
    | _ -> parse_error "plan[:/]"

let rec parse_plan_message () =
  let rec work ?(result=[]) () =
    match Lexer.next () with
	[ "from:"; src ] ->
	  await_string "plan\\";
	  work ~result:((src, parse_plan_from ()) :: result) ()
      | [ "from/" ] ->
	  result
      | _ -> parse_error "from[:/]"
  in
    await_string "from\\";
    work ()

