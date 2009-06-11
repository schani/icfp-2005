
open World
open Worldmap

type possible_locations = int array

let move_robber = Move_Foot

let get_fixed_locations skel node = 
  let pl = Array.make (skel.ws_num_nodes) 0 in
  pl.(node.node_index) <- 1;
  pl

let get_initial_locations skel = 
  get_fixed_locations skel 
    (List.find (fun x -> x.node_tag = ND_RobberStart) skel.ws_nodes)

let step_possible_locations pl wmap mt =
  let pos = Array.copy pl in
  let ag = select_graph wmap mt in
  let rec loop i = 
    if i = Array.length pl then
      pos
    else (
      if pl.(i) > 0 then
	Adjgraph.iter_neighbours ag i (fun i cost -> pos.(i) <- 1);
      loop (i+1)
    )
  in
  loop 0


let get_smell_locations wmap location dist mt = 
  let sn = Utils.get_smell_nodes wmap location.node_index dist mt in
  let size = Worldmap.size wmap in
  let locs = Array.make size 0 in
  let loop = function
    | [] -> ()
    | sn::sns -> locs.(sn.node_index) <- 1
  in
  loop sn;
  locs

let get_possible_smell_locations wmap location mt = 
  let (||) = Utils.array_int_or in
  let max_smell = match mt with Move_Foot -> 2 | Move_Car -> 1 in
  let loop pl = function
    | i when i > max_smell -> pl
    | i -> pl || (get_smell_locations wmap location i mt) 
  in
  loop (Array.make (Worldmap.size wmap) 0) 1

let get_cop_locations wmsg skel = 
  let size = skel.ws_num_nodes in
  let locs = Array.make size 0 in
  let rec loop = function 
    | [],[] -> locs
    | [],x -> loop (x,[])
    | (bot,node)::rest,y -> locs.(node.node_index) <- 1; loop (rest,y)
  in
  let robbers,foot_cops,car_cops = Utils.split_players wmsg in
  loop (foot_cops,car_cops)

let get_bank_locations wmsg skel = 
  let size = skel.ws_num_nodes in
  let locs = Array.make size 0 in
  let rec loop = function 
    | [] -> locs
    | (bank,value)::bvs -> locs.(bank.node_index) <- 1; loop bvs
  in
  loop wmsg.wm_bankvalue

(*
 * updates robber locations with 
 *  - known location (if player entry for robber in current world)
 *  - updated possible locations: 
 *     - one step from previous locations, 
 *     - positions where i can smell 
 *    but exclude: 
 *          - other cops positions 
 *          - bank positions
 *          - own smell to reduce 
 *)
let update_robber_locations pl robber wmsg skel wmap me = 
  try 
    let robber_loc = Utils.get_player_position wmsg robber in
    get_fixed_locations skel robber_loc
  with
    Not_found -> 
      let (-) = Utils.array_posint_minus in
      let (&&) = Utils.array_int_and in
      let my_node = Utils.get_player_position wmsg me in
      let my_mtype = Utils.mtype_of_ptype (Utils.get_player_ptype wmsg me) in
      let pl = step_possible_locations pl wmap move_robber in
      let pl = 
	if wmsg.wm_smell > 0 then
	  pl && (get_smell_locations wmap my_node wmsg.wm_smell my_mtype) 
	else
	  pl - (get_possible_smell_locations wmap my_node my_mtype)
      in
      let pl = pl - (get_cop_locations wmsg skel) in
      let pl = pl - (get_bank_locations wmsg skel) in
      pl
	
(* fully trusts others, i.e. karin mode 
 * FIXME: rate other cops information
 *)
let use_others_informs skel pl informs = 
  let others_locs = Array.make (skel.ws_num_nodes) 0 in
  
  let rec loop = function
    | [] -> ()
    | info::infos -> 
	others_locs.(info.inf_node.node_index) <- 1;
	loop infos
  in
  Utils.array_int_and pl others_locs 
