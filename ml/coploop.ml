


open World
open Parser
open Unparser

let dbg = Debug.debug_string

let some_or_default default = function
  | Some x -> x
  | None -> default 

let skelref = ref (Obj.magic None)
let wmsgref = ref (Obj.magic None)

let default_inform _ = 
  dbg 100 "default_inform";
  []
let default_plan _ = 
  dbg 100 "default_plan";
  []

(* selfvoter *)
let default_vote _ = 
  dbg 100 "default_vote";
  let s = !skelref in
  s.ws_name::(List.filter (fun x -> x!=s.ws_name) (Array.to_list s.ws_cops)) 

let default_move _ = 
  dbg 100 "default_move";
  let s = !skelref in
  let wm = !wmsgref in
  Utils.get_player_position_ptype wm s.ws_name 


let coploop nametype ws_handler wmsg_to_inform inform_to_plan plan_to_vote vote_to_move game_over_handler = 
  let rec loop () = 
    let wmsg = parse_world_message () in
    match wmsg with 
    | None -> some_or_default ignore game_over_handler ()
    | Some(wmsg) -> (
	wmsgref := wmsg;
	let info = some_or_default default_inform wmsg_to_inform wmsg in
	let ____ = unparse_informs info in
	let info = parse_inform_message () in
	let plan = some_or_default default_plan inform_to_plan info in
	let ____ = unparse_plan plan in
	let plan = parse_plan_message () in
	let vote = some_or_default default_vote plan_to_vote plan in
	let ____ = unparse_voting vote in
	let vote = parse_vote_tally () in
	let move = some_or_default default_move vote_to_move vote in
	let ____ = unparse_move move in
	loop ()
      )
  in
  
  dbg 100 "starting";

  

  let ____ = unparse_register nametype in
  let skel = parse_world_skeleton () in
  skelref := skel;  
  dbg 100 "got skel";
  let ____ = some_or_default ignore ws_handler skel in
  loop ()



