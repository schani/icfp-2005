
open World
open Worldmap
open Coploop
open RobberTrackingUtils

let dbg = Debug.debug_string 0

let skel = ref (Obj.magic None)
let wmap = ref (Obj.magic None)
let locs = ref (Obj.magic None)

let wmsg = ref {wm_loot= 0; wm_world= 0; wm_bankvalue= []; wm_evidence= []; 
		wm_smell= 0; wm_players= []}


let set_skel s = 
  skel := s;
  wmap := build_worldmap s;
  locs := get_initial_locations s


let set_wmsg w = 
  wmsg := w 

let wmsg_to_inform wmsg =
  let prev_locs = !locs in
  let robber = (!skel).ws_robbers.(0) in
  locs := update_robber_locations
    !locs !skel.ws_robbers.(0) wmsg !skel !wmap (!skel).ws_name;
  let count_poss = (Array.fold_left (fun cnt x -> 
				      if x > 0 then cnt + 1 else cnt
				   )
		      0 prev_locs) in
  let template = {inf_player= robber; 
		  inf_node= (node_get 1); 
		  inf_ptype= PTYPE_Robber; 
		  inf_world= wmsg.wm_world;
		  inf_certainty= 100/count_poss;
		 }
  in
  if count_poss < Constants.inform_others_threshold then
    let lst = ref [] in
    Array.iteri (fun index v -> 
		   if v > 0 then  
		       lst := {template with 
				 inf_node = (node_get index);
				 inf_certainty= 100/count_poss
			      }::!lst
		     else
		       ()
(* ommiting negative info *)
(* 		       {template with  *)
(* 			  inf_node = (node_get index); *)
(* 			  inf_certainty= -100/((Array.length !locs) - count_poss) *)
(* 		       } *)
		  ) 
	 !locs;
    !lst
  else
    []

let inform_to_plan informs = 
  
  let karin_locs = use_others_informs !skel !locs informs in
  []
  
    
    
      


let _ = 
  coploop ("resi",PTYPE_Cop_Foot) 
    (Some set_skel) (Some wmsg_to_inform) (Some inform_to_plan) None None None

