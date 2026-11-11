import Link from "next/link";
import { record, recordPhotos } from "@/data/portfolio";
import { SectionHead } from "@/components/site/section-head";

/** Record: sticky photo column beside a ranked linked list. */
export function RecordList() {
  return (
    <section className="region" id="record" aria-labelledby="record-h">
      <SectionHead region="record" id="record-h" title="Record" lede="Competition results, most of them team efforts." />
      <div className="rec">
        <div className="rphotos rv">
          {recordPhotos.map((p) => (
            <figure className="rphoto" data-r="clip" key={p.src}>
              {/* eslint-disable-next-line @next/next/no-img-element -- static export */}
              <img src={p.src} alt={p.alt} width={p.width} height={p.height} loading="lazy" />
              <figcaption>{p.caption}</figcaption>
            </figure>
          ))}
        </div>
        <ol className="rlist rv">
          {record.map((r) => (
            <li className="ri" data-r key={r.title}>
              <span className="big" aria-label={r.label}>{r.small && <small>{r.small}</small>}{r.rank}{r.sup && <sup>{r.sup}</sup>}</span>
              <div><h3>{r.title}</h3><p>{r.text}{r.link && <> <Link href={r.link.href}>{r.link.label}</Link></>}</p></div>
            </li>
          ))}
        </ol>
      </div>
    </section>
  );
}
