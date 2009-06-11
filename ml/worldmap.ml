(* $Id: worldmap.ml,v 1.6 2005/07/09 13:47:25 schani Exp $
 *
 * worldmap - something produced during ICFP Contest 2005
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

(* the worldmap API:
 * 
 * General remarks:
 *    - paths include src and dst
 *    - distance is the number of edges between them
 * 
 * 
 * val build_worldmap: World.world_skeleton -> int worldmap
 *   create a worldmap from a given world_skeleton. 
 * 
 * val get_distance: 'a worldmap -> World.node -> World.node 
 *                               -> moveby -> int
 *   returns the distance from the first node to the second
 *   for this kind of movement. 
 * 
 * val get_path : 'a worldmap -> World.node -> World.node  
 *                            -> moveby -> World.node list
 *   returns the shortes path from first to second as a list 
 *   (forward direction)
 * 
 * val map_path : 'a worldmap -> World.node -> World.node 
 *                        -> moveby -> (World.node -> 'b) -> 'b list
 *   map over the path from first to second node in forward dir. 
 * 
 * val iter_path :'a worldmap -> World.node -> World.node 
 *                       -> moveby -> (World.node -> 'b) -> unit
 * 
 * <don't use in Elseware:>
 * 
 * val select_graph : worldmap -> moveby -> int Adjgraph.t
 *   return the adjgraph corresponding to Move_Car or Move_Foot 
 *   movement. only use in extreme cases :-)
 *   
 * 
 * 
 *)
open World

type moveby = 
  | Move_Foot 
  | Move_Car

type 'a worldmap = ('a Adjgraph.t * 'a Adjgraph.t)
let make_graph size = Adjgraph.create size

  
let build_worldmap skel = 
  let foot_graph = make_graph skel.ws_num_nodes in
  let car_graph = make_graph skel.ws_num_nodes in

  let rec edge_loop = function
    | [] -> ()
    | e::es -> 
	let src = e.edge_from.node_index in
	let dst = e.edge_to.node_index in 
	let cost = 1 in begin
	  match e.edge_type with 
	  | ET_Car -> 
	      begin
		Adjgraph.add_edge car_graph src dst cost;
		edge_loop es
	      end
	  | ET_Foot ->
	      begin
		Adjgraph.add_edge car_graph src dst cost;
		
		(* foot always bidirectional *)
		Adjgraph.add_edge foot_graph src dst cost;
		Adjgraph.add_edge foot_graph dst src cost;
		edge_loop es
	      end
	end
  in 
  List.iter (fun n -> edge_loop n.node_edges) skel.ws_nodes;
  (foot_graph,car_graph)

let select_graph (foot,car) = function
  | Move_Foot -> foot
  | Move_Car -> car

let size = function 
    (g,_) -> g.Adjgraph.ag_size
  
let get_distance wm src dst t = 
    let g = select_graph wm t in
    Adjgraph.get_distance g src.node_index dst.node_index 

let get_nodes_with_dist wm dst distance t = 
  let g = select_graph wm t in
  let dists = Adjgraph.get_distances g dst in
  let rec loop acc i = 
    if i = Array.length dists then
      acc
    else
      let acc = if dists.(i) = distance then i::acc else acc in
      loop acc (i+1)
  in
  List.map (fun i -> node_get i) (loop [] 0)

let get_nodes_with_dists wm dst distances t = 
  let g = select_graph wm t in
  let dists = Adjgraph.get_distances g dst in
  let size = size wm in
  let rec loop acc i = 
    if i = size then
      acc
    else
      let acc = if List.mem dists.(i) distances then i::acc else acc in
      loop acc (i+1)
  in
  List.map (fun i -> node_get i) (loop [] 0)

let get_path wm src dst t = 
  let g = select_graph wm t in
  List.rev_map
    (fun i -> (node_get i))
    (Adjgraph.fold_path g src.node_index dst.node_index 
      (fun l x -> x::l ) [])
    
let map_path wm src dst t f = 
  let g = select_graph wm t in
  List.map 
    (fun i -> f (node_get i))
    (List.rev
      (Adjgraph.fold_path g src.node_index dst.node_index
      (fun l x -> x::l ) []))


let iter_path wm src dst t f = 
  ignore(map_path wm src dst t (fun x -> ignore(f x)))


let is_valid_move wm src dst t = 
  let g = select_graph wm t in
  Adjgraph.has_edge g src.node_index dst.node_index

exception Implement_me

let is_cul_de_sac wskel map node =
  let compute () =
    let num_occs = Array.make wskel.ws_num_nodes 0
    and is_sac = ref false
    in List.iter (fun n ->
		    if n != node then
		      List.iter (fun m ->
				   if m != node && m != n then
				     num_occs.(m.node_index) <- (num_occs.(m.node_index) + 1)
				   else
				     ())
			   (get_path map node n Move_Foot)
		    else
		      ())
	 wskel.ws_nodes ;
      Array.iter (fun num ->
		    if num >= wskel.ws_num_nodes - 10 then
		      is_sac := true
		    else
		      ())
	num_occs ;
      !is_sac
  in
    match node.node_is_cul_de_sac with
	Some b -> b
      | None ->
	  let result = (compute ())
	  in node.node_is_cul_de_sac <- Some result ;
	    result
	 	  
let calc_combined_dists wm dst t = 
  let infty = 1 lsl 29 in 
  let sz = size wm in
  

  let curr_graph = select_graph wm t in 
  let curr_dists = Adjgraph.get_distances curr_graph dst.node_index in 
  
  let switch_point = World.get_hq () in 
  let to_switch_point = Adjgraph.get_distances curr_graph switch_point.node_index
  in 
  
  let other_t =  
    match t with  
      | Move_Car -> Move_Foot 
      | Move_Foot -> Move_Car 
  in 
  let cost_from_switchpoint = 
    get_distance wm switch_point dst other_t in 
  
  let combinded_dists = Array.make sz infty in 
  let combinded_switchp = Array.make sz false in
  
  for i = 0 to (Array.length curr_dists) - 1 do 
    let new_costs = to_switch_point.(i) + cost_from_switchpoint in 
    
    if new_costs > curr_dists.(i) then  
      combinded_dists.(i) <- curr_dists.(i)  
    else (
      combinded_dists.(i) <- new_costs;
      combinded_switchp.(i) <- true;
    )
  done; 
  combinded_dists,combinded_switchp
     


let combined_dist_cache = ref (Array.make 1 None)
let combined_switched_cache = ref (Array.make 1 None)
let combined_cache_initialized = ref false  

 


let init_cache wm = 
  if not !combined_cache_initialized then
    (
      combined_cache_initialized := true;
      combined_dist_cache := Array.make (size wm) None;
      combined_switched_cache := Array.make (size wm) None;
    )

(* fixme cache the stuff *)
let get_combined_dists wm dst t = 
  init_cache wm;
  
  match (!combined_dist_cache).(dst.node_index) with 
    | None -> 
	let dists,switched = calc_combined_dists wm dst t in
	(!combined_dist_cache).(dst.node_index) <- Some(dists);
	(!combined_switched_cache).(dst.node_index) <- Some(switched);
	dists
    | Some(cd) -> 
	cd

let get_combined_dist wm src dst t = 
  (get_combined_dists wm dst t).(src.node_index)
   
let get_combined_switched_p wm src dst t =
  init_cache wm;
  
  match (!combined_switched_cache).(dst.node_index) with 
    | None -> 
	let dists,switched = calc_combined_dists wm dst t in
	(!combined_dist_cache).(dst.node_index) <- Some(dists);
	(!combined_switched_cache).(dst.node_index) <- Some(switched);
	switched 
    | Some(cd) -> 
	cd
