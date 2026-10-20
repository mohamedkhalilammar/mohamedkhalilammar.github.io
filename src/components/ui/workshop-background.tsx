/* eslint-disable @next/next/no-img-element -- Responsive WebP assets are generated for static hosting. */
export function WorkshopBackground({ photo, alt, priority = false }: {
  photo: "opening" | "introduction" | "writing";
  alt: string;
  priority?: boolean;
}) {
  return (
    <div className="workshop-background">
      <img
        src={`/media/optimized/workshop-${photo}-960.webp`}
        srcSet={`/media/optimized/workshop-${photo}-480.webp 480w, /media/optimized/workshop-${photo}-960.webp 960w, /media/optimized/workshop-${photo}-1600.webp 1600w`}
        sizes="(min-width: 1200px) 600px, (min-width: 768px) 50vw, 100vw"
        width={960}
        height={1280}
        alt={alt}
        loading={priority ? "eager" : "lazy"}
        fetchPriority={priority ? "high" : "auto"}
        decoding="async"
      />
    </div>
  );
}
