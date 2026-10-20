/* eslint-disable @next/next/no-img-element -- Responsive assets are pre-generated for the static export. */
interface Props { photos: string[]; }

/** Native scroll-snap gallery on mobile; desktop uses the adjacent photo column. */
export function AchievementPhotoStrip({ photos }: Props) {
  return (
    <div className="achievement-photo-strip lg:hidden" role="region" aria-label="Competition photos" tabIndex={0}>
      {photos.map((src, i) => (
        <img
          key={src}
          src={src.replace("-960.webp", "-480.webp")}
          alt={`CTF competition and team achievement ${i + 1}`}
          width={480}
          height={360}
          loading="lazy"
          decoding="async"
        />
      ))}
    </div>
  );
}
