"use client";

import { useState, useCallback, useEffect, useRef } from "react";

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

  // Use refs to avoid stale closures
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

  // ── AI Turn ───────────────────────────────────────────────
  const doAiTurn = useCallback(() => {
    const current = cardsRef.current;
    const unmatched = current.filter(c => !c.matched);
    if (unmatched.length < 2) return;

    const aiMem = aiMemoryRef.current;
    let pick1 = -1, pick2 = -1;

    // Find a known matching pair
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

    // Otherwise pick two random unseen cards
    if (pick1 === -1) {
      const unseenCards = unmatched.filter(c => {
        const seen = aiMem.get(c.symbol) || [];
        return !seen.includes(c.id);
      });

      if (unseenCards.length > 0) {
        const idx = Math.floor(Math.random() * unseenCards.length);
        pick1 = unseenCards[idx].id;
        const sym1 = unseenCards[idx].symbol;
        // Learn card 1
        const m1 = aiMem.get(sym1) || [];
        if (!m1.includes(pick1)) aiMem.set(sym1, [...m1, pick1]);

        // Check if we now know the partner
        const knownSym = aiMem.get(sym1) || [];
        const partner = knownSym.find(id => id !== pick1 && !current.find(c => c.id === id)?.matched);
        if (partner !== undefined) {
          pick2 = partner;
        } else {
          // Pick another unseen
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
        // All seen; pick any two unmatched
        pick1 = unmatched[0].id;
        pick2 = unmatched[1].id;
      }
    }

    if (pick1 === -1 || pick2 === -1) {
      setIsAiTurn(false);
      return;
    }

    // Animate: flip card 1
    timerRef.current = setTimeout(() => {
      syncCards(cardsRef.current.map(c => c.id === pick1 ? { ...c, flipped: true } : c));

      // Animate: flip card 2
      timerRef.current = setTimeout(() => {
        syncCards(cardsRef.current.map(c => c.id === pick2 ? { ...c, flipped: true } : c));

        // Evaluate
        timerRef.current = setTimeout(() => {
          const c1 = cardsRef.current.find(c => c.id === pick1)!;
          const c2 = cardsRef.current.find(c => c.id === pick2)!;

          if (c1.symbol === c2.symbol) {
            // Match!
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

  // ── Player Flip ───────────────────────────────────────────
  const flip = useCallback((id: number) => {
    if (locked || isAiTurn || statusRef.current !== "playing") return;
    if (flippedRef.current.length >= 2) return;

    const card = cardsRef.current.find(c => c.id === id);
    if (!card || card.flipped || card.matched) return;

    // Teach AI about this card
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

  const winner = playerScore > aiScore ? "YOU WIN! 🏆" : playerScore === aiScore ? "DRAW! 🤝" : "AI WINS 🤖";
  const winColor = playerScore > aiScore ? "#34d399" : playerScore === aiScore ? "#f59e0b" : "#f87171";

  return (
    <div className="game-panel">
      <div className="game-hud" style={{ justifyContent: "space-between" }}>
        <span style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: 2 }}>
          <span className="game-label" style={{ fontSize: "0.55rem" }}>YOU</span>
          <span className="game-score" style={{ color: "#f59e0b", fontSize: "1.4rem", lineHeight: 1 }}>{playerScore}</span>
        </span>
        <div style={{ textAlign: "center" }}>
          <span className="game-label" style={{ fontSize: "0.58rem", display: "block", letterSpacing: "0.12em" }}>
            {isAiTurn ? "🤖 AI THINKING…" : status === "playing" ? `MOVE ${moves}` : "MEMORY"}
          </span>
          {bestMoves && <span style={{ fontFamily: "monospace", fontSize: "0.52rem", color: "#a78bfa" }}>BEST {bestMoves} moves</span>}
        </div>
        <span style={{ display: "flex", flexDirection: "column", alignItems: "center", gap: 2 }}>
          <span className="game-label" style={{ fontSize: "0.55rem" }}>AI 🤖</span>
          <span className="game-score" style={{ color: "#f87171", fontSize: "1.4rem", lineHeight: 1 }}>{aiScore}</span>
        </span>
      </div>

      {status === "idle" ? (
        <div style={{ height: 320, display: "flex", flexDirection: "column", alignItems: "center",
          justifyContent: "center", gap: 16, background: "rgba(0,0,0,0.3)", borderRadius: 10,
          border: "1px solid rgba(245,158,11,0.1)" }}>
          <div style={{ textAlign: "center", fontFamily: "monospace", fontSize: "0.78rem",
            color: "rgba(245,158,11,0.6)", textTransform: "uppercase", lineHeight: 2, letterSpacing: "0.1em" }}>
            10 pairs · 20 cards<br />
            <span style={{ fontSize: "0.65rem", color: "rgba(245,158,11,0.35)" }}>
              AI learns every card you flip
            </span>
          </div>
          <button className="game-btn" onClick={init}>PLAY VS AI</button>
        </div>
      ) : (
        <>
          <div style={{ display: "grid", gridTemplateColumns: "repeat(5, 1fr)", gap: 5, marginTop: 4 }}>
            {cards.map(card => {
              const isVisible = card.flipped || card.matched;
              return (
                <div
                  key={card.id}
                  onClick={() => flip(card.id)}
                  style={{
                    aspectRatio: "1",
                    cursor: isVisible || isAiTurn || locked ? "default" : "pointer",
                    perspective: "800px",
                    userSelect: "none",
                  }}
                >
                  <div style={{
                    width: "100%", height: "100%", position: "relative",
                    transformStyle: "preserve-3d",
                    transform: isVisible ? "rotateY(180deg)" : "rotateY(0deg)",
                    transition: "transform 0.38s cubic-bezier(0.16,1,0.3,1)",
                  }}>
                    {/* Back face */}
                    <div style={{
                      position: "absolute", inset: 0, backfaceVisibility: "hidden",
                      background: "linear-gradient(135deg, rgba(245,158,11,0.08), rgba(245,158,11,0.03))",
                      border: "1px solid rgba(245,158,11,0.14)",
                      borderRadius: 6, display: "flex", alignItems: "center", justifyContent: "center",
                    }}>
                      <span style={{ fontSize: "0.75rem", opacity: 0.18, color: "#f59e0b", fontFamily: "monospace", fontWeight: 800 }}>?</span>
                    </div>
                    {/* Front face */}
                    <div style={{
                      position: "absolute", inset: 0, backfaceVisibility: "hidden",
                      transform: "rotateY(180deg)",
                      background: card.matched
                        ? card.matchedBy === "player"
                          ? "linear-gradient(135deg, rgba(52,211,153,0.18), rgba(52,211,153,0.06))"
                          : "linear-gradient(135deg, rgba(248,113,113,0.18), rgba(248,113,113,0.06))"
                        : "linear-gradient(135deg, rgba(245,158,11,0.14), rgba(245,158,11,0.05))",
                      border: `1px solid ${card.matched
                        ? card.matchedBy === "player" ? "rgba(52,211,153,0.55)" : "rgba(248,113,113,0.55)"
                        : "rgba(245,158,11,0.45)"}`,
                      borderRadius: 6, display: "flex", alignItems: "center", justifyContent: "center", flexDirection: "column",
                      boxShadow: card.matched
                        ? card.matchedBy === "player" ? "0 0 12px rgba(52,211,153,0.25)" : "0 0 12px rgba(248,113,113,0.25)"
                        : "none",
                    }}>
                      <span style={{ fontSize: "1.1rem" }}>{card.symbol}</span>
                      {card.matched && (
                        <span style={{
                          fontSize: "0.38rem", fontFamily: "monospace", fontWeight: 800,
                          color: card.matchedBy === "player" ? "rgba(52,211,153,0.9)" : "rgba(248,113,113,0.9)",
                          textTransform: "uppercase", letterSpacing: "0.1em",
                        }}>
                          {card.matchedBy === "player" ? "YOU" : "AI"}
                        </span>
                      )}
                    </div>
                  </div>
                </div>
              );
            })}
          </div>

          {status === "won" && (
            <div style={{ textAlign: "center", marginTop: 14, padding: "1rem 1.5rem",
              background: "rgba(0,0,0,0.5)", borderRadius: 8,
              border: `1px solid ${winColor}44` }}>
              <p style={{ fontFamily: "monospace", fontSize: "1rem", fontWeight: 900,
                color: winColor, textTransform: "uppercase", letterSpacing: "0.2em", marginBottom: 4 }}>
                {winner}
              </p>
              <p style={{ fontFamily: "monospace", fontSize: "0.65rem", color: "rgba(245,158,11,0.45)",
                marginBottom: 12, textTransform: "uppercase" }}>
                {playerScore} — {aiScore} · {moves} moves
              </p>
              <button className="game-btn" onClick={init}>REMATCH</button>
            </div>
          )}
        </>
      )}

      <div className="game-controls-hint">
        {isAiTurn ? "🤖 AI is playing — it remembers what you've flipped!" : "Click cards · Match before the AI does"}
      </div>
    </div>
  );
}
