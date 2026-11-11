"use client";

import { useState } from "react";

/** YouTube demo behind a poster: nothing loads from YouTube until the visitor presses play. */
export function VideoEmbed({ url, title }: { url: string; title: string }) {
  const [playing, setPlaying] = useState(false);
  const id = url.split("/embed/")[1]?.split(/[?&]/)[0] ?? "";
  const src = url + (url.includes("?") ? "&" : "?") + "autoplay=1&rel=0";

  return (
    <div className="vid">
      {playing ? (
        <iframe src={src} title={title} allow="autoplay; encrypted-media; picture-in-picture; fullscreen" allowFullScreen />
      ) : (
        <button type="button" className="vid-poster" onClick={() => setPlaying(true)} aria-label={"Play " + title}>
          {/* eslint-disable-next-line @next/next/no-img-element -- remote thumbnail, static export */}
          {id && <img src={`https://i.ytimg.com/vi/${id}/hqdefault.jpg`} alt="" loading="lazy" />}
          <span className="vid-play" aria-hidden="true"><svg viewBox="0 0 24 24"><path d="M8 5.5v13l10.5-6.5z" /></svg></span>
          <span className="vid-lab mono">play demo</span>
        </button>
      )}
    </div>
  );
}
