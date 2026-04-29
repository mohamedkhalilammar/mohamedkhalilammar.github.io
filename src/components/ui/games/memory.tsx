"use client";

import { useState, useCallback, useEffect, useRef } from "react";
import { motion, AnimatePresence } from "framer-motion";

const SYMBOLS = ["⚡", "🔥", "💀", "👾", "🛸", "🔮", "💎", "🎯", "🧬", "🌀"];

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
      syncCards(cardsRef.current.map(c => c.id === pick1 ? { ...c, flipped: true } : c));

      timerRef.current = setTimeout(() => {
        syncCards(cardsRef.current.map(c => c.id === pick2 ? { ...c, flipped: true } : c));

        timerRef.current = setTimeout(() => {
          const c1 = cardsRef.current.find(c => c.id === pick1)!;
          const c2 = cardsRef.current.find(c => c.id === pick2)!;

          if (c1.symbol === c2.symbol) {
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

  const init = useCallback(() => {
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
              <span className="text-[9px] font-mono text-amber-500/30 uppercase font-bold tracking-widest">PLAYER</span>
              <span className="text-2xl font-black text-amber-500/80 font-mono tracking-tighter">{playerScore}</span>
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
                            : "rgba(245,158,11,0.2)"
                        }}
                      >
                        <span className="text-2xl">{card.symbol}</span>
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
