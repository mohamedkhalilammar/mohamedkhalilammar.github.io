"use client";

/* eslint-disable @next/next/no-img-element -- Static hosting uses pre-generated responsive photos. */
import { useCallback, useEffect, useRef, useState } from "react";

const PHOTOS = [
  { name: "Speaking", file: "workshop-writing", alt: "Khalil speaking at the lectern during a workshop on AI and cybersecurity.", portrait: true },
  { name: "Cybersphere", file: "cybersphere", alt: "Khalil and his teammates holding their first-place prize at Cybersphere Congress.", portrait: false },
  { name: "CTF finals", file: "finals", alt: "Khalil and his team working together during the CTF finals.", portrait: false },
  { name: "Competition", file: "photo", alt: "Khalil holding competition prizes at a cybersecurity event.", portrait: false },
];

/** Decoded photo crossfades with optional automatic rotation. */
export function CreativeEntrance4D() {
  const [current, setCurrent] = useState(0);
  const [paused, setPaused] = useState(false);
  const [pending, setPending] = useState<number | null>(null);
  const [photoError, setPhotoError] = useState(false);
  const images = useRef<(HTMLImageElement | null)[]>([]);
  const requested = useRef(0);
  const touchStart = useRef<{ x: number; y: number } | null>(null);
  const section = useRef<HTMLElement | null>(null);
  const selectPhoto = useCallback(async (index: number) => {
    requested.current = index;
    setPending(index);
    setPhotoError(false);
    try {
      const image = images.current[index];
      if (!image) throw new Error("Photo unavailable");
      image.loading = "eager";
      if (!image.getAttribute("src")) {
        const file = PHOTOS[index].file;
        image.srcset = `/media/optimized/${file}-480.webp 480w, /media/optimized/${file}-960.webp 960w, /media/optimized/${file}-1600.webp 1600w`;
        image.src = `/media/optimized/${file}-1600.webp`;
      }
      await image.decode();
      if (requested.current === index) setCurrent(index);
    } catch {
      if (requested.current === index) setPhotoError(true);
    } finally {
      if (requested.current === index) setPending(null);
    }
  }, []);
  useEffect(() => {
    if (paused) return;
    const motion = window.matchMedia("(prefers-reduced-motion: reduce)");
    const timer = window.setInterval(() => {
      const bounds = section.current?.getBoundingClientRect();
      if (motion.matches || document.hidden || !bounds || bounds.bottom <= 0 || bounds.top >= window.innerHeight) return;
      void selectPhoto((requested.current + 1) % PHOTOS.length);
    }, 6000);
    return () => window.clearInterval(timer);
  }, [paused, current, selectPhoto]);
  const advance = (direction: number) => { void selectPhoto((requested.current + direction + PHOTOS.length) % PHOTOS.length); };

  return (
    <section
      id="entrance"
      ref={section}
      aria-label="Introduction"
      className="portfolio-entrance"
      onMouseEnter={() => setPaused(true)}
      onMouseLeave={() => setPaused(false)}
      onFocusCapture={() => setPaused(true)}
      onBlurCapture={event => { if (!event.currentTarget.contains(event.relatedTarget)) setPaused(false); }}
      onTouchStart={event => { touchStart.current = { x: event.touches[0].clientX, y: event.touches[0].clientY }; }}
      onTouchEnd={event => {
        if (!touchStart.current) return;
        const dx = touchStart.current.x - event.changedTouches[0].clientX;
        const dy = touchStart.current.y - event.changedTouches[0].clientY;
        if (Math.abs(dx) > 50 && Math.abs(dx) > Math.abs(dy)) advance(dx > 0 ? 1 : -1);
        touchStart.current = null;
      }}
      onTouchCancel={() => { touchStart.current = null; }}
    >
      <div className="entrance-backgrounds">
        {PHOTOS.map((photo, index) => (
          <img
            key={photo.file}
            ref={element => { images.current[index] = element; }}
            src={index === 0 ? `/media/optimized/${photo.file}-1600.webp` : undefined}
            srcSet={index === 0 ? `/media/optimized/${photo.file}-480.webp 480w, /media/optimized/${photo.file}-960.webp 960w, /media/optimized/${photo.file}-1600.webp 1600w` : undefined}
            sizes={photo.portrait ? "(min-width: 768px) 65vw, 100vw" : "100vw"}
            width={960}
            height={photo.portrait ? 1280 : 720}
            alt={photo.alt}
            aria-hidden={index !== current}
            data-active={index === current}
            className={photo.portrait ? "entrance-photo entrance-photo--portrait" : "entrance-photo"}
            fetchPriority={index === 0 ? "high" : "low"}
            loading={index === 0 ? "eager" : "lazy"}
            decoding="async"
            draggable={false}
          />
        ))}
      </div>
      <div className="entrance-inner">
        <div className="entrance-copy">
          <h1><span>Khalil</span><span>Ammar</span></h1>
          <p className="entrance-role">Cybersecurity enthusiast<br />&amp; engineering student at INSAT.</p>
          <a className="entrance-work-link" href="#projects">Explore my work <span aria-hidden>↗</span></a>
        </div>
        <div className="entrance-controls" role="group" aria-label="Background photos" aria-busy={pending !== null}>
          <div className="entrance-photo-tabs">
            {PHOTOS.map((photo, index) => (
              <button type="button" key={photo.file} aria-label={`Show ${photo.name}`} aria-pressed={current === index} onClick={() => { void selectPhoto(index); }}><span aria-hidden /></button>
            ))}
          </div>
          <div className="entrance-photo-arrows">
            <button type="button" onClick={() => advance(-1)} aria-label="Previous photo">←</button>
            <button type="button" onClick={() => advance(1)} aria-label="Next photo">→</button>
          </div>
          <p className={photoError ? "entrance-photo-error" : "sr-only"} role="status">{photoError ? "Photo unavailable. Please choose another." : pending !== null ? "Loading photo…" : PHOTOS[current].alt}</p>
        </div>
      </div>
    </section>
  );
}
