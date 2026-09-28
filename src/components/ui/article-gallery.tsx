"use client";

import { useEffect, useRef, useState } from "react";
import { createPortal } from "react-dom";

export type ArticleImage = { src: string; alt: string };

export function ArticleGallery({ images, layout = "carousel" }: { images: ArticleImage[]; layout?: "carousel" | "row" }) {
  const [active, setActive] = useState(0);
  const [current, setCurrent] = useState<number | null>(null);
  const [zoomed, setZoomed] = useState(false);
  const dialogRef = useRef<HTMLDialogElement>(null);
  const triggerRef = useRef<HTMLButtonElement | null>(null);
  const open = current !== null;

  useEffect(() => {
    if (!open) return;
    const dialog = dialogRef.current;
    const trigger = triggerRef.current;
    dialog?.showModal();
    const overflow = document.body.style.overflow;
    document.body.style.overflow = "hidden";
    return () => {
      dialog?.close();
      document.body.style.overflow = overflow;
      trigger?.focus({ preventScroll: true });
    };
  }, [open]);

  const close = () => { setCurrent(null); setZoomed(false); };
  const move = (step: number) => {
    setCurrent(index => index === null ? null : (index + step + images.length) % images.length);
    setZoomed(false);
  };

  return <>
    <div className={"writeup-gallery" + (layout === "row" ? " writeup-gallery-row" : "")}>
      {(layout === "row" ? images.map((image, index) => ({ image, index })) : [{ image: images[active], index: active }]).map(({ image, index }) => <figure key={image.src + index}>
        <button type="button" className="writeup-image-trigger" aria-label={"Enlarge " + (image.alt || "screenshot")} onClick={event => { triggerRef.current = event.currentTarget; setCurrent(index); }}>
          <img src={image.src} alt={image.alt} loading="lazy" />
          <span className="writeup-image-expand" aria-hidden>Expand ↗</span>
        </button>
        <figcaption aria-live={layout === "carousel" ? "polite" : undefined}><span>{image.alt}</span>{layout === "carousel" && <span>{index + 1} / {images.length}</span>}</figcaption>
      </figure>)}
      {layout === "carousel" && images.length > 1 && <div className="writeup-thumbnail-strip" role="group" aria-label="Choose screenshot">
        {images.map((image, index) => <button key={image.src + index} type="button" aria-label={"Show " + (image.alt || "screenshot " + (index + 1))} aria-pressed={active === index} onClick={event => { setActive(index); event.currentTarget.scrollIntoView({ block: "nearest", inline: "nearest" }); }}>
          <img src={image.src} alt="" loading="lazy" />
          <span>{index + 1}</span>
        </button>)}
      </div>}
    </div>
    {current !== null && createPortal(<dialog ref={dialogRef} className="writeup-image-dialog" aria-label="Screenshot viewer" onCancel={close} onClick={event => { if (event.target === event.currentTarget) close(); }} onKeyDown={event => {
      if (event.key === "ArrowRight") { event.preventDefault(); move(1); }
      if (event.key === "ArrowLeft") { event.preventDefault(); move(-1); }
    }}>
      <div className="writeup-viewer-toolbar">
        <span aria-live="polite">{current + 1} / {images.length}</span>
        <div>
          <button type="button" aria-pressed={zoomed} onClick={() => setZoomed(value => !value)}>{zoomed ? "Fit to screen" : "Zoom in"}</button>
          <a href={images[current].src} target="_blank" rel="noopener noreferrer">Original ↗</a>
          <button type="button" aria-label="Close screenshot viewer" onClick={close}>✕</button>
        </div>
      </div>
      <div className="writeup-viewer-image" data-zoomed={zoomed}><img src={images[current].src} alt={images[current].alt} /></div>
      <div className="writeup-viewer-footer">
        <button type="button" aria-label="Previous screenshot" disabled={images.length < 2} onClick={() => move(-1)}>←</button>
        <p>{images[current].alt}</p>
        <button type="button" aria-label="Next screenshot" disabled={images.length < 2} onClick={() => move(1)}>→</button>
      </div>
    </dialog>, document.body)}
  </>;
}
