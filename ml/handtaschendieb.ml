(* $Id: handtaschendieb.ml,v 1.2 2005/07/09 13:53:01 schani Exp $
 *
 * handtaschendieb - something produced during ICFP Contest 2005
 * Copyright (C) 2005  Mark Probst
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

exception Timeout

open World
open Worldmap
open Parser
open Unparser

let debug_on = true

type mode = Mode_evade | Mode_loot

type position_score = {
  ps_node: node;
  ps_cul_de_sac_penalty :int;
  ps_smell_penalty :int;
  ps_cop_penalty :int;
  ps_bank_refuel_penalty :int;
  ps_bank_loot_bonus :int;
  ps_bank_evade_penalty :int;
  ps_degree_penalty :int;
  ps_bank_distance_bonus :int;
}

(* bank refules ist a list of (node * int) *)
let bank_refuels = ref []

let num_robbers      = 1
let num_cops         = 5

let num_banks          = 6
let max_bank_refuels   = 5
let total_bank_money   = 6000

let inf_score          = 99999999

let bank_value_scale        = 4
let bank_dist_value_scale   = 3
let smell_penalty           = 1000
let degree_penalty          = 5000
let cul_de_sac_penalty      = 15000
let bank_refuel_penalty     = 5000
let bank_evade_penalty      = 8000

let cop_minus_2_penalty   = 20000
let cop_minus_1_penalty   = 15000
let cop_0_penalty         = 10000
let cop_1_penalty         = 3000
let cop_2_penalty         = 1000
let cop_3_penalty         = 300
let cop_4_penalty         = 100

let evasion_timeout    = 8

let max_moves          = 200
let robber_start_pos   = "54-and-ridgewood"

(* robber recursion control *)
let robber_short_recursion   = 5
let robber_long_recursion    = 7
let robber_minimax_depth     = 1

let dmsg msg =
  if debug_on then begin
    output_string stderr msg;
    flush stderr
  end

let advance_bank_refules () =
  bank_refuels :=
    List.filter (fun _, t -> if t <= 0 then false else true)
      (List.map (fun n, t -> n, t - 1) !bank_refuels)

let register_bank_refuel (node: node) t =
  bank_refuels := (node, t) :: !bank_refuels

let get_bank_refuel node =
  try
    List.assoc node !bank_refuels
  with Not_found -> 0

(* returns (node * ptype) of the player with this name
 * may raise Not_found
 *)
let player_node world_msg name =
  List.assoc name world_msg.wm_players

let opt_invert_choose choose invert =
  match choose, invert with
      Move_Foot, true -> Move_Car
    | Move_Car, true -> Move_Foot
    | x, false -> x

let cop_penalty = function
    x when x <= -2 -> cop_minus_2_penalty
  | -1 -> cop_minus_1_penalty
  | 0 -> cop_0_penalty
  | 1 -> cop_1_penalty
  | 2 -> cop_2_penalty
  | 3 -> cop_3_penalty
  | _ -> cop_4_penalty

let calc_best_position_score () =
  let loot_score = bank_value_scale * total_bank_money / 5
  and distance_score = bank_dist_value_scale * total_bank_money / 5
  in
    loot_score + distance_score

let total_pols_score ps =
  0 - ps.ps_cul_de_sac_penalty - ps.ps_smell_penalty -
    ps.ps_cop_penalty - ps.ps_bank_refuel_penalty + ps.ps_bank_loot_bonus -
    ps.ps_bank_evade_penalty - ps.ps_degree_penalty + ps.ps_bank_distance_bonus

let best_position_score = calc_best_position_score ()
let cop_contact_ago = ref 0
let mode = ref Mode_loot
let cop_contact_ago = ref 0

let sum_map f l =
  List.fold_right (fun x sum -> sum + (f x)) l 0

(* todo: complete me *)
(* returns (int * position_score)
 *)
let position_score wskel wmsg map my_node cop_nodes future is_end_point =
  let bank_sum = sum_map snd wmsg.wm_bankvalue
  in let smell_penalty () =
    0
  in
    { ps_node = my_node;
      ps_cul_de_sac_penalty = if is_cul_de_sac wskel map my_node then
	cul_de_sac_penalty / (future * future)
      else
	0;
      ps_smell_penalty = 0; (* todo *)
      ps_cop_penalty = 0;
      ps_bank_refuel_penalty =
	sum_map (fun (n, bv) ->
		   if (get_bank_refuel n) > 0 then
		     bank_refuel_penalty / ((get_distance map n my_node Move_Foot) + 1)
		   else
		     0) wmsg.wm_bankvalue ;
      ps_bank_loot_bonus =
	if !mode = Mode_loot then
	  sum_map (fun (n, bv) ->
		     if n = my_node
		       && (not (is_cul_de_sac wskel map n))
		       && (get_bank_refuel n) < future then
			 bank_value_scale * bank_sum / 5
		     else
		       0) wmsg.wm_bankvalue
	else
	  0 ;
      ps_bank_evade_penalty =
	if !mode = Mode_evade then
	  sum_map (fun (n, bv) ->
		     if n = my_node then
		       bank_evade_penalty
		     else
		       0) wmsg.wm_bankvalue
	else
	  0 ;
      ps_degree_penalty =
	if is_end_point then
	  let degree = node_foot_degree my_node
	  in degree_penalty / (degree * degree)
	else
	  0 ;
      ps_bank_distance_bonus =
	if is_end_point then
	  sum_map (fun (n, bv) ->
		     if is_cul_de_sac wskel map n then
		       0
		     else
		       let dist = get_distance map my_node n Move_Foot
		       in if dist > 0 then
			   bank_dist_value_scale * bank_sum / 5 / dist
			 else
			   0) wmsg.wm_bankvalue
	else
	  0 ;
    }

(* returns 1 of bot can smell me, or 0 if not *)
let cop_can_smell_me map my_node a_robot =
  match a_robot with
      _, (bot_node, PTYPE_Cop_Foot) ->
	if get_distance map bot_node my_node Move_Foot <= 2 then 1 else 0
    | _, (bot_node, PTYPE_Cop_Car) ->
	if get_distance map bot_node my_node Move_Car <= 1 then 1 else 0
    | _ -> 0

(* returns the numbers of bots that can smell me *)
let cops_can_smell_me wmsg map my_node =
  List.fold_left (+) 0
    (List.map (cop_can_smell_me map my_node) wmsg.wm_players)

let check_cop_smell wmsg map my_node =
  if 0 < (cops_can_smell_me wmsg map my_node) then begin
    cop_contact_ago := 1;
    dmsg "alert: cops can smell me!\n";
  end

(* todo: write me, probably finish position_score first.. *)
(*
let rec search_best_target wskel wmsg map my_node cop_nodes
    ?(future=0) ?(score_so_far=0) ?(best_score_so_far=(inf_score * -1))
    max_depth ignore_timeout =
  let pos_score, pos_score_info =
    position_score wskel wmsg map my_node cop_nodes (future + 1)
      (future == max_depth -1)
  in
    if not ignore_timeout then
      timeout_test;
    if (score_so_far + pos_score) > best_score_so_far then
      if future < (max_depth - 1) then
	rec_node
*)

let print_pos_score ps =
  dmsg (Printf.sprintf "node                %s\n" ps.ps_node.node_loc);
  dmsg (Printf.sprintf "  cul-de-sac penalty      %d\n"
	  ps.ps_cul_de_sac_penalty);
  dmsg (Printf.sprintf "  smell penalty           %d\n" ps.ps_smell_penalty);
  dmsg (Printf.sprintf  "  cop penalty             %d\n" ps.ps_cop_penalty);
  dmsg (Printf.sprintf  "  bank refuel penalty     %d\n"
	  ps.ps_bank_refuel_penalty);
  dmsg (Printf.sprintf  "  bank loot bonus         %d\n"
	  ps.ps_bank_loot_bonus);
  dmsg (Printf.sprintf  "  bank evade penalty      %d\n"
	  ps.ps_bank_evade_penalty);
  dmsg (Printf.sprintf  "  degree penalty          %d\n"
	  ps.ps_degree_penalty);
  dmsg (Printf.sprintf "  bank distance bonus     %d\n"
	  ps.ps_bank_distance_bonus);
  dmsg (Printf.sprintf  "  TOTAL                   %d\n\n"
	  (total_pols_score ps))

let _ =
  dmsg (Printf.sprintf "best score is %d\n" best_position_score);
  unparse_register ("handtaschendieb", World.PTYPE_Robber);
  let s = parse_world_skeleton ()
  in let map = build_worldmap s
  in let rec moveit () =
      let m = match parse_world_message () with
	  Some m -> m
	| None -> dmsg "game over\n"; exit 0;
      in let my_node = fst (player_node m s.ws_name)
      in
	dmsg (Printf.sprintf "we are at %s\n" my_node.node_loc);
	check_cop_smell m map my_node;
	if !cop_contact_ago <= evasion_timeout then
	  mode := Mode_evade
	else
	  mode := Mode_loot;
  in
    moveit ()
    
    
