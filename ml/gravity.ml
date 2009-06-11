open Worldmap
open World


let infty = 1 lsl 29


let calc_bank_gravity ws wm map = 
  let size =  (Worldmap.size map) in
  let gravity = Array.make size infty in
  let best_bank = Array.make size (World.get_hq (), 0, infty) in
  let g = Worldmap.select_graph map Move_Foot in
  
  let bank_distances_list = 
    let rec loop = function
      | [] -> []
      | (node,worth)::bvs -> 
	  let dists = Adjgraph.get_distances g node.node_index in
	  (node,worth,dists)::(loop bvs)
    in
    loop wm.wm_bankvalue
  in
  
  let choose_best_bank index = 
    let rec loop best = function
      | [] -> best_bank.(index) <- best
      | (((node,worth,dists) as bi)::bis) -> 
	  loop 
	    (
	      let bnode,bworth,bdist = best_bank.(index) in
	      if bdist > dists.(index) then (
		best_bank.(index) <- bi;
		bi
	      )
	      else
		bi
	    )
	    bis
    in
    loop (World.get_hq (), 0, infty) bank_distances_list 
  in
  
  for i = 0 to size - 1 do 
    let (node,worth,dists) = best_bank.(i) in
    let best = choose_best_bank i in

    let scale = 2 * dists * dists in
    let gravity = (Constants.empty_bank_value + worth ) / scale in
    gravity.(i) <- gravity
  done;
  gravity
  
let calc_cop_gravity ws wmsg map = 
  let size = Worldmap.size map in
  let gravity = Array.make size infty in
  let g = Worldmap.select_graph map Move_Foot in

  let rec loop index = function
    | [] -> ()
    | (cop,(node,ptype))::cops when ptype != PTYPE_Robber -> 
	let mt = mtype_of_ptype ptype in
	let cop_dist = Worldmap.get_combined_dist map node index mt in
	gravity.(index) = gravity.(index) + 
	    (constants.copweight / cop_dist)
    | robber::cops -> loop cops 
  in
  
  for i = 0 to size - 1 do 
    loop index wmsg.wm_players 
  done;
  gravity
  
  
