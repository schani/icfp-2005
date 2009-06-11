(* $Id: hsim.ml,v 1.4 2005/06/30 20:36:29 heinz Exp $
 *
 * hsim - something produced during ICFP Contest 2005
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

open World
open Parser

let node_radius = 1.2

let gfx_node_color n =
  match n.node_tag with
      ND_HQ -> "blue"
    | ND_Bank -> "yellow"
    | ND_RobberStart -> "brown"
    | ND_Ordinary -> "gray"

let gfx_create_node root n =
  ignore (GnoCanvas.ellipse root ~fill_color:(gfx_node_color n)
	    ~x1:(n.node_x -. node_radius) ~y1:(n.node_y -. node_radius)
	    ~x2:(n.node_x +. node_radius) ~y2:(n.node_y +. node_radius))

let gfx_create_edge root e =
  let fill_color, props =
    match e.edge_type with
	ET_Car -> "red",
	  [`ARROW_SHAPE_B 0.5; `FIRST_ARROWHEAD true; `LAST_ARROWHEAD true;
	   `CAP_STYLE `ROUND; `WIDTH_PIXELS 2]
      | ET_Foot -> "black",
	  [`ARROW_SHAPE_C 0.2; `FIRST_ARROWHEAD true; `LAST_ARROWHEAD true;
	   `CAP_STYLE `ROUND; `WIDTH_PIXELS 1]
  in
    ignore (GnoCanvas.line root ~fill_color ~props
	      ~points:[| e.edge_from.node_x; e.edge_from.node_y;
			 e.edge_to.node_x; e.edge_to.node_y; |])

let zoom_changed canvas adj () =
  canvas#set_pixels_per_unit adj#value

let status_line = ref (GMisc.label ~text:"Statusline" ~justify:`FILL ())

let create_canvas window =
  let vbox = GPack.vbox ~border_width:4 ~spacing:4 ~packing:window#add () in
  let table = GPack.table ~rows:2 ~columns:2 ~packing:vbox#add ()
  in let canvas = GnoCanvas.canvas ~aa:false ~width:1030 ~height:520 () in
  canvas#set_center_scroll_region false;
  canvas#set_scroll_region (-5.0) (-5.0) 200. 95.;
  canvas#scroll_to (-5) (-5);
  let frame = GBin.frame () in
  table#attach ~left:0 ~right:1 ~top:0 ~bottom:1 ~expand:`BOTH ~fill:`BOTH 
    ~shrink:`BOTH ~xpadding:0 ~ypadding:0 frame#coerce;
  frame#add canvas#coerce;
  let w = GRange.scrollbar `HORIZONTAL ~adjustment:canvas#hadjustment () in
  table#attach ~left:0 ~right:1 ~top:1 ~bottom:2
    ~expand:`X ~fill:`BOTH ~shrink:`X ~xpadding:0 ~ypadding:0
    w#coerce ;
  let w = GRange.scrollbar `VERTICAL ~adjustment:canvas#vadjustment () in
  table#attach ~left:1 ~right:2 ~top:0 ~bottom:1
    ~expand:`Y ~fill:`BOTH ~shrink:`Y ~xpadding:0 ~ypadding:0 
    w#coerce;
  let hbox = GPack.hbox ~spacing:4 ~packing:vbox#pack () in
  let adj = GData.adjustment 
      ~value:5. ~lower:0.05 ~upper:20. 
      ~step_incr:0.05 ~page_incr:0.5 ~page_size:0.5 () in
  ignore (adj#connect#value_changed (zoom_changed canvas adj));
  ignore (GEdit.spin_button ~adjustment:adj ~rate:0. ~digits:2 ~width:50 
	    ~packing:hbox#pack ());
  let b = GButton.button ~stock:`OPEN ~label:"Load World" ~packing:hbox#pack ()
  in
(*  ignore (b#connect#clicked (load_file window canvas)); *)
  let b = GButton.button ~label:"Reset" ~packing:hbox#pack ()
  in
(*  ignore (b#connect#clicked (reset_game canvas#root)); *)
  let b = GButton.button ~label:"+1" ~packing:hbox#pack ()
  in
(*  ignore (b#connect#clicked (perform_step 1)); *)
  let b = GButton.button ~label:"+5" ~packing:hbox#pack ()
  in
(*  ignore (b#connect#clicked (perform_step 5)); *)
  let b = GButton.button ~label:"Update" ~packing:hbox#pack ()
  in
  hbox#pack !status_line#coerce;
  canvas

let ws = parse_world_skeleton ()

let _ =
  try
    let window = GWindow.window ()
    in let canvas = create_canvas window
    in
      ignore (window#connect#destroy ~callback:GMain.Main.quit);
      List.iter (gfx_create_node canvas#root) ws.ws_nodes;
      List.iter (gfx_create_edge canvas#root) ws.ws_edges;
      window#show ();
      GMain.Main.main ()
  with
      Exit -> ()
