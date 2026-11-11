"use client";

import { useCallback, useEffect, useRef, useState } from "react";
import { heroIntro, heroPhotos } from "@/data/portfolio";
import { MOTION_EVENT, readMotion } from "@/lib/motion";

const ROTATE_MS = 7000;

/** Full-bleed photo backdrop that crossfades every 7s while motion is on. */
export function Hero() {
  const [shown, setShown] = useState(0);
  const [cycle, setCycle] = useState(0);
  const timer = useRef(0);

  const schedule = useCallback(() => {
    window.clearTimeout(timer.current);
    const auto = readMotion() && !document.hidden && !document.documentElement.classList.contains("still");
    document.documentElement.classList.toggle("photos-paused", !auto);
    if (auto) timer.current = window.setTimeout(() => { setShown((n) => (n + 1) % heroPhotos.length); setCycle((c) => c + 1); }, ROTATE_MS);
  }, []);

  useEffect(() => {
    schedule();
    return () => window.clearTimeout(timer.current);
  }, [shown, cycle, schedule]);

  useEffect(() => {
    window.addEventListener(MOTION_EVENT, schedule);
    document.addEventListener("visibilitychange", schedule);
    return () => { window.removeEventListener(MOTION_EVENT, schedule); document.removeEventListener("visibilitychange", schedule); };
  }, [schedule]);

  const pick = (n: number) => { setShown(n); setCycle((c) => c + 1); };

  return (
    <section className="hero" id="entry" aria-labelledby="name">
      <div className="hero-bg" id="herobg">
        {heroPhotos.map((p, n) => (
          // eslint-disable-next-line @next/next/no-img-element -- static export, photos are pre-sized webp
          <img key={p.src} className={n === shown ? "on" : undefined} src={p.src} alt={p.alt} width={p.width} height={p.height}
            style={{ objectPosition: p.position }} aria-hidden={n === shown ? undefined : true} fetchPriority={n === 0 ? "high" : undefined} />
        ))}
      </div>
      <div className="hero-copy">
        <h1 className="name" id="name" aria-label="Khalil Ammar"><span className="w"><span>Khalil</span></span> <span className="w"><span>Ammar</span></span></h1>
        <p className="aka">a.k.a. <b>fadigaa</b></p>
        <p className="role">{heroIntro}</p>
        <div className="ctas">
          <a href="#writing">Read the writeups</a>
          <a href="#contact">Get in touch</a>
        </div>
      </div>
      <div className="hero-sw" id="herosw" role="group" aria-label="Background photo">
        {heroPhotos.map((p, n) => (
          <button key={p.src + cycle} type="button" aria-label={"Photo " + (n + 1)} aria-pressed={n === shown} onClick={() => pick(n)}>
            <i /><span>{String(n + 1).padStart(2, "0")}</span>
          </button>
        ))}
      </div>
    </section>
  );
}
