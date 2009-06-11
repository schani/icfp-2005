
open World
open Worldmap

let mtype_of_ptype = function
  | PTYPE_Robber
  | PTYPE_Cop_Foot 
    -> Move_Foot
  | PTYPE_Cop_Car 
    -> Move_Car

let etype_of_ptype = function
  | PTYPE_Robber
  | PTYPE_Cop_Foot 
    -> ET_Foot
  | PTYPE_Cop_Car 
    -> ET_Car
      
let etype_of_mtype = function 
  | Move_Car -> ET_Car
  | Move_Foot -> ET_Foot
      
let mtype_of_etype = function
  | ET_Car -> Move_Car
  | ET_Foot -> Move_Foot



let get_player_position wmsg bot = 
  let node,ptype = List.assoc bot wmsg.wm_players in
  node

let get_player_ptype wmsg bot = 
  let node,ptype = List.assoc bot wmsg.wm_players in
  ptype

let get_player_position_ptype wmsg bot = 
  List.assoc bot wmsg.wm_players 

let get_neighbours skel node the_edge_type = 
  List.filter (fun e -> e.edge_type = the_edge_type) node.node_edges

let split_players wmsg = 
  let rec loop bots acc = 
    match bots with
    | [] -> acc 
    | ((bot,(node,ptype))::bots) ->
	let robbers,foot_cops,car_cops = acc in
	match ptype with 
	| PTYPE_Robber   -> loop bots ((bot,node)::robbers,foot_cops,car_cops)
	| PTYPE_Cop_Foot -> loop bots (robbers,(bot,node)::foot_cops,car_cops)
	| PTYPE_Cop_Car  -> loop bots (robbers,foot_cops,(bot,node)::car_cops)
  in
  loop (wmsg.wm_players) ([],[],[])
	

let get_players_by_ptype wmsg ptype =
  let pred = function
    | (bot,(n,pt)) -> pt = ptype 
  in
  List.filter pred wmsg.wm_players

let get_smell_nodes wmap location dist mt = 
  get_nodes_with_dist wmap location dist mt

exception Incompatible_sizes

let array_map2 a1 a2 f = 
  let l1 = Array.length a1 in
  let l2 = Array.length a2 in
  if l1 != l2 then
    raise Incompatible_sizes
  else
    let f i a1' = f a1' a2.(i) in
    Array.mapi f a1 
    

let array_int_mul a1 a2 = 
  let f a1' a2' = 
    a1'*a2'
  in
  array_map2 a1 a2 f

let array_int_or a1 a2 = 
  let f a1' a2' = 
    if a1' * a2' > 0 then 1 else 0
  in
  array_map2 a1 a2 f
    
let array_int_and a1 a2 = 
  let f a1' a2' = 
    if a1' + a2' > 0 then 1 else 0
  in
  array_map2 a1 a2 f
    
let array_posint_minus a1 a2 = 
  let minus x y = if y > x then 0 else x-y in
  array_map2 a1 a2 minus


