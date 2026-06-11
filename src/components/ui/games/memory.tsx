"use client";

import { useState, useCallback, useEffect, useRef, type ReactNode } from "react";
import { motion, AnimatePresence } from "framer-motion";
import { initAudio, sfx } from "./audio";

/** Ten distinct SVG glyphs, each with its own color — replaces the emoji deck. */
const GLYPHS: Record<string, { color: string; icon: ReactNode }> = {
  bolt: {
    color: "#fbbf24",
    icon: <path d="M13 2L4 14h6l-1 8 9-12h-6l1-8z" />,
  },
  flame: {
    color: "#fb7185",
    icon: <path d="M12 2c1.5 4 5 5.5 5 10a5 5 0 11-10 0c0-1.8.8-3 1.8-4.6.5 1.6 1.7 2.1 1.7 2.1C9.8 6.8 11 4.6 12 2z" />,
  },
  skull: {
    color: "#e4e4e7",
    icon: (
      <>
        <path d="M12 3a7 7 0 00-7 7c0 2.8 1.8 4.2 2 5.6V19h10v-3.4c.2-1.4 2-2.8 2-5.6a7 7 0 00-7-7z" />
        <circle cx="9.4" cy="10.5" r="1.5" fill="#0a0c18" />
        <circle cx="14.6" cy="10.5" r="1.5" fill="#0a0c18" />
      </>
    ),
  },
  bug: {
    color: "#34d399",
    icon: (
      <>
        <ellipse cx="12" cy="13" rx="5" ry="6" />
        <circle cx="12" cy="6" r="2.5" />
        <path stroke="#34d399" strokeWidth="1.5" strokeLinecap="round" fill="none" d="M7 10L4 8M7 14H3.5M7.5 17L5 19.5M17 10l3-2M17 14h3.5M16.5 17l2.5 2.5" />
      </>
    ),
  },
  rocket: {
    color: "#22d3ee",
    icon: <path d="M12 2c3 2.2 4.2 6 4.2 9.2l2.3 3.4-3.4-1c-.8 1.9-5.4 1.9-6.2 0l-3.4 1 2.3-3.4C7.8 8 9 4.2 12 2zm0 6.5a1.6 1.6 0 100 3.2 1.6 1.6 0 000-3.2z" />,
  },
  gem: {
    color: "#a78bfa",
    icon: <path d="M7 3h10l4 5.5L12 21 3 8.5 7 3z" />,
  },
  target: {
    color: "#f87171",
    icon: (
      <>
        <circle cx="12" cy="12" r="9" fill="none" stroke="#f87171" strokeWidth="2" />
        <circle cx="12" cy="12" r="5" fill="none" stroke="#f87171" strokeWidth="2" />
        <circle cx="12" cy="12" r="1.8" />
      </>
    ),
  },
  wave: {
    color: "#60a5fa",
    icon: <path fill="none" stroke="#60a5fa" strokeWidth="2.2" strokeLinecap="round" d="M2 12c2.5-5 5-5 7.5 0s5 5 7.5 0 3-4 5-1" />,
  },
  star: {
    color: "#fde047",
    icon: <path d="M12 2l2.6 6.6L21.5 9.3l-5 4.6 1.6 6.9L12 17.2 5.9 20.8l1.6-6.9-5-4.6 6.9-.7z" />,
  },
  moon: {
    color: "#c4b5fd",
    icon: <path d="M20.5 13.5A8.5 8.5 0 1110.5 3.5a7 7 0 0010 10z" />,
  },
};

const SYMBOLS = Object.keys(GLYPHS);

function makeFullDeck() {
  return [...SYMBOLS, ...SYMBOLS];
}

function shuffle<T>(arr: T[]): T[] {
  const a = [...arr];
  for (let i = a.length - 1; i > 0; i--) {
    const j = Math.floor(Math.random() * (i + 1));
    [a[i], a[j]] = [a[j], a[i]];
  }
  return a;
}

type Card = { id: number; symbol: string; flipped: boolean; matched: boolean; matchedBy: "player" | "ai" | null };
type Status = "idle" | "playing" | "won";
type AIMemory = Map<string, number[]>;

export function MemoryGame() {
  const [cards, setCards] = useState<Card[]>([]);
  const [status, setStatus] = useState<Status>("idle");
  const [playerScore, setPlayerScore] = useState(0);
  const [aiScore, setAiScore] = useState(0);
  const [moves, setMoves] = useState(0);
  const [locked, setLocked] = useState(false);
  const [isAiTurn, setIsAiTurn] = useState(false);
  const [bestMoves, setBestMoves] = useState<number | null>(null);
  const timerRef = useRef<ReturnType<typeof setTimeout> | null>(null);

  const cardsRef = useRef<Card[]>([]);
  const flippedRef = useRef<number[]>([]);
  const aiMemoryRef = useRef<AIMemory>(new Map());
  const playerScoreRef = useRef(0);
  const aiScoreRef = useRef(0);
  const movesRef = useRef(0);
  const statusRef = useRef<Status>("idle");

  const totalPairs = SYMBOLS.length;

  const syncCards = (updated: Card[]) => {
    cardsRef.current = updated;
    setCards(updated);
  };

  const doAiTurn = useCallback(() => {
    const current = cardsRef.current;
    const unmatched = current.filter(c => !c.matched);
    if (unmatched.length < 2) return;

    const aiMem = aiMemoryRef.current;
    let pick1 = -1, pick2 = -1;

    for (const [sym, ids] of aiMem.entries()) {
      const avail = ids.filter(id => {
        const c = current.find(cc => cc.id === id);
        return c && !c.matched;
      });
      if (avail.length >= 2) {
        pick1 = avail[0];
        pick2 = avail[1];
        break;
      }
    }

    if (pick1 === -1) {
      const unseenCards = unmatched.filter(c => {
        const seen = aiMem.get(c.symbol) || [];
        return !seen.includes(c.id);
      });

      if (unseenCards.length > 0) {
        const idx = Math.floor(Math.random() * unseenCards.length);
        pick1 = unseenCards[idx].id;
        const sym1 = unseenCards[idx].symbol;
        const m1 = aiMem.get(sym1) || [];
        if (!m1.includes(pick1)) aiMem.set(sym1, [...m1, pick1]);

        const knownSym = aiMem.get(sym1) || [];
        const partner = knownSym.find(id => id !== pick1 && !current.find(c => c.id === id)?.matched);
        if (partner !== undefined) {
          pick2 = partner;
        } else {
          const remaining = unseenCards.filter(c => c.id !== pick1);
          if (remaining.length > 0) {
            const ri = Math.floor(Math.random() * remaining.length);
            pick2 = remaining[ri].id;
            const sym2 = remaining[ri].symbol;
            const m2 = aiMem.get(sym2) || [];
            if (!m2.includes(pick2)) aiMem.set(sym2, [...m2, pick2]);
          } else {
            const fallback = unmatched.filter(c => c.id !== pick1);
            pick2 = fallback[Math.floor(Math.random() * fallback.length)]?.id ?? -1;
          }
        }
      } else {
        pick1 = unmatched[0].id;
        pick2 = unmatched[1].id;
      }
    }

    if (pick1 === -1 || pick2 === -1) {
      setIsAiTurn(false);
      return;
    }

    timerRef.current = setTimeout(() => {
      sfx("flip");
      syncCards(cardsRef.current.map(c => c.id === pick1 ? { ...c, flipped: true } : c));

      timerRef.current = setTimeout(() => {
        sfx("flip");
        syncCards(cardsRef.current.map(c => c.id === pick2 ? { ...c, flipped: true } : c));

        timerRef.current = setTimeout(() => {
          const c1 = cardsRef.current.find(c => c.id === pick1)!;
          const c2 = cardsRef.current.find(c => c.id === pick2)!;

          if (c1.symbol === c2.symbol) {
            sfx("aiMatch");
            syncCards(cardsRef.current.map(c =>
              c.id === pick1 || c.id === pick2 ? { ...c, matched: true, matchedBy: "ai", flipped: false } : c
            ));
            const ns = aiScoreRef.current + 1;
            aiScoreRef.current = ns;
            setAiScore(ns);

            if (ns + playerScoreRef.current >= totalPairs) {
              statusRef.current = "won";
              setStatus("won");
              setIsAiTurn(false);
            } else {
              timerRef.current = setTimeout(() => doAiTurn(), 700);
            }
          } else {
            timerRef.current = setTimeout(() => {
              syncCards(cardsRef.current.map(c =>
                c.id === pick1 || c.id === pick2 ? { ...c, flipped: false } : c
              ));
              setIsAiTurn(false);
              setLocked(false);
            }, 500);
          }
        }, 700);
      }, 650);
    }, 850);
  }, [totalPairs]);

  const flip = useCallback((id: number) => {
    if (locked || isAiTurn || statusRef.current !== "playing") return;
    if (flippedRef.current.length >= 2) return;

    const card = cardsRef.current.find(c => c.id === id);
    if (!card || card.flipped || card.matched) return;

    const m = aiMemoryRef.current.get(card.symbol) || [];
    if (!m.includes(id)) aiMemoryRef.current.set(card.symbol, [...m, id]);

    const newFlipped = [...flippedRef.current, id];
    flippedRef.current = newFlipped;

    sfx("flip");
    syncCards(cardsRef.current.map(c => c.id === id ? { ...c, flipped: true } : c));

    if (newFlipped.length === 2) {
      setLocked(true);
      const nm = movesRef.current + 1;
      movesRef.current = nm;
      setMoves(nm);

      const [a, b] = newFlipped;
      const cardA = cardsRef.current.find(c => c.id === a)!;
      const cardB = cardsRef.current.find(c => c.id === b)!;

      if (cardA.symbol === cardB.symbol) {
        timerRef.current = setTimeout(() => {
          sfx("match");
          syncCards(cardsRef.current.map(c =>
            c.id === a || c.id === b ? { ...c, matched: true, matchedBy: "player", flipped: false } : c
          ));
          flippedRef.current = [];
          const ns = playerScoreRef.current + 1;
          playerScoreRef.current = ns;
          setPlayerScore(ns);

          if (ns + aiScoreRef.current >= totalPairs) {
            statusRef.current = "won";
            setStatus("won");
            setBestMoves(best => {
              const mv = movesRef.current;
              return best === null || mv < best ? mv : best;
            });
          }
          setLocked(false);
        }, 300);
      } else {
        timerRef.current = setTimeout(() => {
          sfx("mismatch");
          syncCards(cardsRef.current.map(c =>
            c.id === a || c.id === b ? { ...c, flipped: false } : c
          ));
          flippedRef.current = [];
          setLocked(false);
          setIsAiTurn(true);
          timerRef.current = setTimeout(() => doAiTurn(), 400);
        }, 900);
      }
    }
  }, [locked, isAiTurn, totalPairs, doAiTurn]);

  // end-of-game jingle based on who won
  useEffect(() => {
    if (status !== "won") return;
    const t = setTimeout(() => {
      sfx(playerScoreRef.current >= aiScoreRef.current ? "win" : "lose");
    }, 250);
    return () => clearTimeout(t);
  }, [status]);

  const init = useCallback(() => {
    initAudio();
    sfx("ui");
    if (timerRef.current) clearTimeout(timerRef.current);
    const deck = shuffle(makeFullDeck()).map((symbol, i) => ({
      id: i, symbol, flipped: false, matched: false, matchedBy: null,
    })) as Card[];
    cardsRef.current = deck;
    flippedRef.current = [];
    aiMemoryRef.current = new Map();
    playerScoreRef.current = 0;
    aiScoreRef.current = 0;
    movesRef.current = 0;
    statusRef.current = "playing";
    setCards(deck);
    setStatus("playing");
    setPlayerScore(0);
    setAiScore(0);
    setMoves(0);
    setLocked(false);
    setIsAiTurn(false);
  }, []);

  useEffect(() => () => { if (timerRef.current) clearTimeout(timerRef.current); }, []);

  const winner = playerScore > aiScore ? "YOU WIN!" : playerScore === aiScore ? "DRAW!" : "AI WINS";
  const winColor = playerScore > aiScore ? "#34d399" : playerScore === aiScore ? "#f59e0b" : "#f87171";

  return (
    <div className="w-full max-w-[650px] mx-auto">
      <div className="relative">
        <div className="absolute -top-12 left-0 right-0 z-40 pointer-events-none flex justify-between items-end px-2">
           <div className="flex flex-col">
              <span className="text-[9px] font-mono text-primary-400/30 uppercase font-bold tracking-widest">PLAYER</span>
              <span className="text-2xl font-black text-primary-400/80 font-mono tracking-tighter">{playerScore}</span>
           </div>
           <div className="flex flex-col items-center">
              <span className="text-[8px] font-mono text-zinc-500 uppercase tracking-[0.2em] mb-1">
                {isAiTurn ? "AI MOVING" : status === "playing" ? `MOVE: ${moves}` : "MEMORY"}
              </span>
           </div>
           <div className="flex flex-col items-end">
              <span className="text-[9px] font-mono text-red-500/30 uppercase font-bold tracking-widest">AI</span>
              <span className="text-2xl font-black text-red-500/80 font-mono tracking-tighter">{aiScore}</span>
           </div>
        </div>

        <div className="relative p-1 rounded-xl bg-white/[0.02] border border-white/5 shadow-2xl">
          <div className="grid grid-cols-5 gap-3">
            {cards.length === 0 ? (
              <div className="col-span-5 h-[350px] flex items-center justify-center">
                 <button className="btn-primary !py-3 !px-12 !text-xs" onClick={init}>START GAME</button>
              </div>
            ) : (
              cards.map(card => {
                const isVisible = card.flipped || card.matched;
                return (
                  <div
                    key={card.id}
                    onClick={() => flip(card.id)}
                    className="aspect-square relative"
                    style={{ perspective: "1000px" }}
                  >
                    <div 
                      className="w-full h-full relative transition-transform duration-500" 
                      style={{ 
                        transformStyle: "preserve-3d",
                        transform: isVisible ? "rotateY(180deg)" : "rotateY(0deg)",
                        cursor: isVisible || isAiTurn || locked ? "default" : "pointer" 
                      }}
                    >
                      <div className="absolute inset-0 backface-hidden bg-white/[0.03] border border-white/5 rounded-lg flex items-center justify-center">
                        <span className="text-zinc-700 font-mono text-xs opacity-50">?</span>
                      </div>
                      <div 
                        className={`absolute inset-0 backface-hidden bg-white/[0.08] border rounded-lg flex flex-col items-center justify-center`}
                        style={{ 
                          transform: "rotateY(180deg)",
                          borderColor: card.matched 
                            ? card.matchedBy === "player" ? "rgba(52,211,153,0.3)" : "rgba(239,68,68,0.3)"
                            : "rgba(129,140,248,0.2)"
                        }}
                      >
                        <svg viewBox="0 0 24 24" className={`w-7 h-7 ${card.matched ? "mem-pop" : ""}`} fill={GLYPHS[card.symbol].color} style={{ filter: card.matched ? `drop-shadow(0 0 6px ${GLYPHS[card.symbol].color})` : undefined }} aria-hidden>
                          {GLYPHS[card.symbol].icon}
                        </svg>
                        {card.matched && (
                          <span className={`text-[7px] font-mono font-black mt-1 uppercase ${card.matchedBy === "player" ? "text-green-500/80" : "text-red-500/80"}`}>
                            {card.matchedBy === "player" ? "YOU" : "AI"}
                          </span>
                        )}
                      </div>
                    </div>
                  </div>
                );
              })
            )}
          </div>

          <AnimatePresence>
            {status === "won" && (
              <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }}
                   className="absolute inset-0 bg-black/40 backdrop-blur-md flex flex-col items-center justify-center text-center p-8 rounded-xl z-50">
                <h2 className={`text-4xl font-black uppercase tracking-tighter mb-4 italic`} style={{ color: winColor }}>
                  {winner}
                </h2>
                <p className="text-[10px] font-mono text-zinc-500 mb-8 uppercase tracking-widest">
                  {playerScore} — {aiScore} · {moves} moves
                </p>
                <button className="btn-primary !py-3 !px-10 !text-xs" onClick={init}>TRY AGAIN</button>
              </motion.div>
            )}
          </AnimatePresence>
        </div>
      </div>
    </div>
  );
}
