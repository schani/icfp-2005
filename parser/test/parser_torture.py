#!/usr/bin/env python

# pseudo randomly creates messages according to the icfp2005 grammar

MAX_TK_LEN=100
#MAX_REPEAT=1000
MAX_REPEAT=4
CHARSET="-_#()"+"012345679"+"abcdefghijklmnopqrstuvwxyz"+"ABCDEFGHIJKLMNOPQRSTUVWXYZ"

from random import *

def randint2(a,b):
    "randint with more weight on boundary values"
    return choice([a,randint(a,b),b])

def gen_tk_name():
    def tk_name():
        n = randint2(1, MAX_TK_LEN)
        s = ""
        for _ in xrange(n):
            s += choice(CHARSET)
        return s
    return tk_name

def gen_tk_number(upper_bound, lower_bound = 0):
    def tk_number():
        n = randint2(lower_bound, upper_bound)
        s = str(n)
        avail_padding=MAX_TK_LEN-len(s)
        padding = choice([avail_padding, 1, 0])
        if s[0]=='-':
            s=s[0]+(padding*'0')+s[1:]
        else:
            s=(padding*'0')+s

        assert len(s) <= 100
        return s

    return tk_number

def gen_tk_bot(): return gen_tk_name()
def gen_tk_loc(): return gen_tk_bot()

#

def gen_tk_bank_value(): return gen_tk_number(1000)

def gen_tk_loot(): return gen_tk_number(6000)

def gen_tk_world(): return gen_tk_number(200)

def gen_tk_distance(): return gen_tk_number(1000) # count(nodes) assumed to be 1000

def gen_tk_certainty(): return gen_tk_number(100,-100)

def gen_tk_coordinate(): return gen_tk_number(1023)

#

def gen_tk_sep(): return lambda: choice(["\t"," "])

def gen_tk_eol(): return lambda: choice(["\r\n","\n"])

def gen_tk_ptype(): return lambda: choice(["cop-foot","cop-car","robber"])

def gen_tk_edge_type(): return lambda: choice(["cop-foot","cop-car","robber"])

def gen_tk_node_tag(): return lambda: choice(["hq","bank","robber-start","ordinary"])

def gen_tk_choice(*tk): return lambda *a: choice(tk)(*a)

#
def gen_tk_str(s):
    return lambda: s

def gen_tk_rand_repeat(tk, maxn=MAX_REPEAT):
    def tk_rand_repeat():
        n = randint2(0, maxn)
        s = ""
        for _ in xrange(n):
            s += tk()
        return s

    return tk_rand_repeat

    
        

# grammar lines
def gen_tuple (*tokens):
    return lambda: "".join(map(apply, tokens))

def gen_tuple_sep (*tokens):
    ntk = [tokens[0]]
    for x in tokens[1:]:
        ntk.append(gen_tk_sep())
        ntk.append(x)
    ntk = tuple(ntk)
    return gen_tuple(*ntk)

def gen_line (*tokens):
    return gen_tuple_sep(*(list(tokens)+[gen_tk_eol()]))


def gen_tk_tagged(tag, *tk):
    return gen_tuple_sep (gen_tk_str(tag+':'), *tk)

def gen_tk_tagged_eol(tag, *tk):
    return gen_tk_tagged (tag, *(list(tk)+[gen_tk_eol()]))

def gen_tk_rand_container(tag, tk, maxn=MAX_REPEAT):
    return gen_tuple(gen_line (gen_tk_str(tag+'\\')),
                     gen_tk_rand_repeat(gen_tk_tagged (tag, tk), maxn),
                     gen_line (gen_tk_str(tag+'/')))


#

def gen_wsk():
    return gen_tuple(
        gen_line(gen_tk_str("wsk\\")),
        gen_tk_tagged_eol("name", gen_tk_bot()),
        gen_tk_tagged_eol("robber", gen_tk_bot()),
        gen_tk_tagged_eol("cop", gen_tk_bot()),
        gen_tk_tagged_eol("cop", gen_tk_bot()),
        gen_tk_tagged_eol("cop", gen_tk_bot()),
        gen_tk_tagged_eol("cop", gen_tk_bot()),
        gen_tk_tagged_eol("cop", gen_tk_bot()),

        gen_tk_rand_container("nod", gen_line(gen_tk_loc(),
                                              gen_tk_node_tag(),
                                              gen_tk_coordinate(),
                                              gen_tk_coordinate())),


        gen_tk_rand_container("edg", gen_line(gen_tk_loc(),
                                              gen_tk_loc(),
                                              gen_tk_edge_type())),
        
        gen_line(gen_tk_str("wsk/"))
        )
    
def gen_wor():
    return gen_tuple(
        gen_line(gen_tk_str("wor\\")),
        gen_tk_tagged_eol("wor", gen_tk_world()),
        gen_tk_tagged_eol("rbd", gen_tk_loot()),


        gen_tk_rand_container("bv", gen_line(gen_tk_loc(),
                                             gen_tk_bank_value())),


        gen_tk_rand_container("ev", gen_line(gen_tk_loc(),
                                             gen_tk_world())),


        gen_tk_tagged_eol("smell", gen_tk_distance()),

        gen_tk_rand_container("pl", gen_line(gen_tk_bot(),
                                             gen_tk_loc(),
                                             gen_tk_ptype())),
        
        gen_line(gen_tk_str("wor/"))
        )
    
    
def gen_game_over():
    return gen_line(gen_tk_str("game-over"))

def gen_vote_tally():
    return gen_tk_choice(gen_tk_tagged_eol("winner", gen_tk_bot()),
                         gen_tk_tagged_eol("nowinner"))

## TODO, messages from other cops
import sys

def main():
    messages = [
        gen_wsk(),
        gen_wor(),
        gen_game_over(),
        gen_vote_tally()
    ]

    try:
        while True:
            for m in messages:
                sys.stdout.write(m())
    except KeyboardInterrupt:
        sys.stderr.write("CTRL-C pressed, shutting down\n")
    except IOError, msg:
        sys.stderr.write("I/O Error (%s) occured, shutting down\n" % msg)

if __name__ == '__main__':
    main()

    
    
