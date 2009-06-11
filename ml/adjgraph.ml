(* $Id: adjgraph.ml,v 1.6 2005/07/05 19:48:18 signof Exp $
 *
 * adjgraph - something produced during ICFP Contest 2005
 * Copyright (C) 2005 Martin Biely
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


type 'a adjgraph = {
  ag_matrix: 'a array array;
(*   ag_adj_lists: 'a list array;  *)
  ag_size: int;
  ag_nolink: 'a; 
  ag_add: 'a -> 'a -> 'a;
  ag_le: 'a -> 'a -> bool;
  ag_zero_cost: 'a;
  ag_infinity: 'a;
  ag_dijkstra_calculated: bool;
  mutable ag_dist_cache: 'a array option array;
  mutable ag_path_cache: int array option array;
}

type 'a t = 'a adjgraph

let intfinity = 1 lsl 29

let make size nolink add le zerocost infinity = 
  {
    ag_matrix=Array.make_matrix size size nolink; 
(*     ag_adj_lists=Array.make size []; *)
    ag_size = size;
    ag_add=add; 
    ag_le=le; 
    ag_nolink=nolink; 
    ag_zero_cost=zerocost;
    ag_infinity = infinity;
    ag_dijkstra_calculated = false;
    ag_dist_cache=Array.make size None; 
    ag_path_cache=Array.make size None  }

let create ?(nolink=0) ?(add=(+)) ?(le=(<=)) ?(zerocost=0) ?(infinity=intfinity) size =
    make size nolink add le zerocost infinity 

let has_edge g src dst =
  (g.ag_matrix.(src).(dst) != g.ag_nolink)


let throw_away_dijkstra g = 
  if g.ag_dijkstra_calculated then begin
    g.ag_dist_cache <- Array.create g.ag_size None; 
    g.ag_path_cache <- Array.create g.ag_size None
  end;
  ()

let add_edge g src dst cost = 
  throw_away_dijkstra g; 
  if not (has_edge g src dst) then (
    g.ag_matrix.(src).(dst) <- cost;
(*     g.ag_adj_lists.(src) <- dst::g.ag_adj_lists.(src); *)
    ()
  )
  else
    ()

let del_edge g src dst = 
  throw_away_dijkstra g; 
  if not (has_edge g src dst) then (
    g.ag_matrix.(src).(dst) <- g.ag_nolink;
(*     g.ag_adj_lists.(src) <- List.filter (fun x -> x!= dst) g.ag_adj_lists.(src) *)
    ()
  )
  else
    ()



(* 
 * dijkstra may be single source, but here I implement single
 * destination. since this allows pi (see e.g. corman) to contain
 * links in forward (i.e. in a useful) direction.
 *)

let calc_dijkstra g dst = 
  
  (* util functions *)
  
  let create_dists () = (
    let dists = Array.make g.ag_size g.ag_infinity in
    dists.(dst) <- g.ag_zero_cost;
    dists
  )
  in
  let create_paths () = (
    let paths = Array.make g.ag_size (-1) in
    paths.(dst) <- dst;
    paths
  )
  in
  let create_pq dst = 
    let pq = Pqueue.create g.ag_le in
    Pqueue.insert pq g.ag_zero_cost dst
  in

  let fold_over_nodes_to dst f a = 
    let rec loop from a = 
      if from < g.ag_size then 	
	begin
	  let a' = 
	    if g.ag_matrix.(from).(dst) != g.ag_nolink then 
	      (f from a)
	    else
	      a
	  in
	  loop (from + 1) a'
	end
      else
	a
    in
    loop 0 a
  in


  (* initializing *)
  
  let dists = create_dists () in
  let paths = create_paths () in
  let relaxed  = Array.make g.ag_size false in

  (* main functions *)

  let relax farer closer  = 
    (* closer is (possibly) closer to dst, farer will (possibly) be
       the predecessor on the path that is to be constructed. *)
    let dist' = g.ag_add dists.(closer) g.ag_matrix.(farer).(closer)
    in
    if g.ag_le dist' dists.(farer) then 
      begin
	dists.(farer) <- dist';
	paths.(farer) <- closer;
	true
      end 
    else
      false
  in
  
  let rec loop q = 
    if Pqueue.emptyP q then (
      (dists, paths))
    else
      let (p,v,q') = Pqueue.extract q in
      let q' = 
	if not relaxed.(v) then 
	  begin
	    relaxed.(v) <- true;
	    fold_over_nodes_to v (fun x q'' ->
	      if relax x v then 
		Pqueue.insert q'' dists.(x) x
	      else 
		q''
	    ) q'
	  end
	else 
	  q'
      in 
      loop q'
  in 
  let q = (create_pq dst) in
  loop q
    

let do_dijkstra g dst = 
  let (dist,path) = calc_dijkstra g dst in
  g.ag_dist_cache.(dst) <- Some(dist);
  g.ag_path_cache.(dst) <- Some(path);
  ()

let rec get_distances g dst = 
  let d = g.ag_dist_cache.(dst) in
  match d with 
  | Some(d) -> d
  | None -> 
      do_dijkstra g dst;
      get_distances g dst

let get_distance g src dst = 
  (get_distances g dst).(src) 

let rec get_path_array g dst = 
  let d = g.ag_path_cache.(dst) in
  match d with 
  | Some(d) -> d
  | None -> 
      do_dijkstra g dst; 
      get_path_array g dst
	
let get_path_next g src dst = 
  (get_path_array g dst).(src)

let fold_path g src dst f a = 
  let p = get_path_array g dst in
  let rec loop v a = 
    if v = dst then 
      f a v
    else
      loop p.(v) (f a v)
  in 
  loop src a

let iter_neighbours g src f = 
  let rec loop i = 
    if i = g.ag_size then
      ()
    else (
      if g.ag_matrix.(src).(i) != g.ag_nolink then
	f i g.ag_matrix.(src).(i);
      loop (i+1)
    )
  in
  loop 0
	
