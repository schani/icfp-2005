(* $Id: world.ml,v 1.13 2005/07/09 13:50:57 heinz Exp $
 *
 * world - something produced during ICFP Contest 2005
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

type node_tag =
    ND_HQ | ND_Bank | ND_RobberStart | ND_Ordinary

type edgetype =
    ET_Car | ET_Foot

type ptype =
    PTYPE_Cop_Foot | PTYPE_Cop_Car | PTYPE_Robber

type bot = string

type node = {
  node_loc: string;
  node_tag: node_tag;
  node_x: float;
  node_y: float;
  node_index: int;
  mutable node_edges: edge list;

  (* robber stuff *)
  mutable node_foot_degree: int option;
  mutable node_is_cul_de_sac: bool option;
}
and edge = {
  edge_from: node;
  edge_to: node;
  edge_type: edgetype;
}

type world_skeleton = {
  ws_name: string;
  ws_robbers: bot array;
  ws_cops: bot array;
  ws_nodes: node list;
  ws_edges: edge list;
  ws_num_nodes: int;
}

type world_message = {
  wm_loot: int;
  wm_world: int;
  wm_bankvalue: (node * int) list;
  wm_evidence: (node * int) list;
  wm_smell: int;
  wm_players: (bot * (node * ptype)) list;
}

type inform = {
  inf_player: bot;
  inf_node: node;
  inf_ptype: ptype;
  inf_world: int;
  inf_certainty: int;
}
    
type plan = {
  plan_player: bot;
  plan_node: node;
  plan_ptype: ptype;
  plan_world: int
}

let node_hash: (string, node) Hashtbl.t = Hashtbl.create 521
let index_hash: (int, node) Hashtbl.t = Hashtbl.create 521
let hq_ref = ref None

let node_register n =
  if n.node_tag = ND_HQ then
    hq_ref := Some(n);
  Hashtbl.add node_hash n.node_loc n;
  Hashtbl.add index_hash n.node_index n

let node_find loc =
  Hashtbl.find node_hash loc

let node_get index = 
  Hashtbl.find index_hash index

let node_foot_degree n =
  let compute () =
    List.length (List.map (fun e -> e.edge_type == ET_Foot) n.node_edges)
  in
    match n.node_foot_degree with
	Some i -> i
      | None ->
	  let result = compute ()
	  in
	    n.node_foot_degree <- Some result;
	    result

let get_hq () = 
  match !hq_ref with 
    | None -> failwith "HQ not initialized"
    | Some(n) -> n
