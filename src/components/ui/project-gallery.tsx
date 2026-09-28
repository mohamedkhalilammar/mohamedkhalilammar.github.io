"use client";

import Image from "next/image";
import { createPortal } from "react-dom";
import { useEffect, useRef, useState } from "react";

interface ProjectGalleryProps { name: string; screenshots: string[]; screenshotCaptions?: string[]; }

export function ProjectGallery({ name, screenshots, screenshotCaptions }: ProjectGalleryProps) {
  const [current, setCurrent] = useState<number | null>(null);
  const dialogRef = useRef<HTMLDialogElement>(null);
  const triggerRef = useRef<HTMLButtonElement | null>(null);
  const isOpen = current !== null;
  useEffect(() => {
    if (!isOpen) return;
    const dialog = dialogRef.current;
    const trigger = triggerRef.current;
    dialog?.showModal();
    const overflow = document.body.style.overflow;
    document.body.style.overflow = "hidden";
    return () => { dialog?.close(); document.body.style.overflow = overflow; trigger?.focus(); };
  }, [isOpen]);
  const move = (delta: number) => setCurrent(i => i === null ? null : (i + delta + screenshots.length) % screenshots.length);
  if (!screenshots.length) return null;
  return (
    <>
      <div className="project-gallery">
        {screenshots.map((src, i) => <figure key={src}>
          <button type="button" aria-label={"Enlarge " + (screenshotCaptions?.[i] || name + " screenshot " + (i + 1))} onClick={event => { triggerRef.current = event.currentTarget; setCurrent(i); }}>
            <Image src={src} alt={screenshotCaptions?.[i] || name + " screenshot " + (i + 1)} width={600} height={400} />
            <span aria-hidden>↗</span>
          </button>
          {screenshotCaptions?.[i] && <figcaption>{screenshotCaptions[i]}</figcaption>}
        </figure>)}
      </div>
      {current !== null && createPortal(
        <dialog ref={dialogRef} className="gallery-dialog" aria-label={name + " image gallery"} onCancel={() => setCurrent(null)} onClick={e => { if (e.target === e.currentTarget) setCurrent(null); }} onKeyDown={e => { if (e.key === "ArrowRight") { e.preventDefault(); move(1); } if (e.key === "ArrowLeft") { e.preventDefault(); move(-1); } }}>
          <div className="gallery-dialog-bar"><span aria-live="polite">Image {current + 1} of {screenshots.length}</span><button type="button" className="ui-icon-button" aria-label="Close image gallery" onClick={() => setCurrent(null)}>×</button></div>
          <div className="gallery-image"><Image src={screenshots[current]} alt={screenshotCaptions?.[current] || name + " screenshot " + (current + 1)} width={1600} height={1000} /></div>
          <div className="gallery-dialog-bottom">
            <button type="button" className="ui-icon-button" aria-label="Previous image" onClick={() => move(-1)} disabled={screenshots.length < 2}>←</button>
            <p>{screenshotCaptions?.[current] || name}</p>
            <button type="button" className="ui-icon-button" aria-label="Next image" onClick={() => move(1)} disabled={screenshots.length < 2}>→</button>
          </div>
        </dialog>, document.body
      )}
    </>
  );
}
