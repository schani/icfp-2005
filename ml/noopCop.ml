


let use_skel skel = 
  let cdist = Worldmap.get_combined_dist
    (Worldmap.build_worldmap skel)
    (World.node_find "57-and-university")
    (World.node_find "51-and-woodlawn")
    Worldmap.Move_Foot
  in
  Printf.fprintf stderr "cdist is %d" cdist
  
let _ = Coploop.coploop ("ocaml_nop",World.PTYPE_Cop_Foot) (Some use_skel) None None None None None
