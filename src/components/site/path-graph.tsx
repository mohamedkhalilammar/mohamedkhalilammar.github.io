import Link from "next/link";
import { interests, pathIntro, pathNodes } from "@/data/portfolio";
import { frameLabel } from "@/lib/trace";
import { Glyph } from "@/components/site/glyph";
import { SectionHead } from "@/components/site/section-head";

/** Path: a plain statement, then an IDA-style graph from Khalil to the three places he works through. */
export function PathGraph() {
  return (
    <section className="region" id="path" aria-labelledby="path-h">
      <SectionHead region="path" id="path-h" title="Path" />
      <div className="say-text rv"><p data-r>{pathIntro}</p></div>
      <div className="graph rv">
        <div className="gtop">
          <div className="gn root" data-r>
            <p className="gl mono">{frameLabel(0)}</p>
            <div className="gb"><h3>Khalil Ammar</h3><p>Engineering student, networks and telecommunications.</p></div>
          </div>
        </div>
        <div className="gcols">
          {pathNodes.map((node, i) => (
            <div className={"gcol " + node.edge} key={node.name}>
              <article className="gn" data-r>
                <p className="gl mono">{frameLabel(i + 1)}</p>
                <div className="gb">
                  <div className="oh">
                    {/* eslint-disable-next-line @next/next/no-img-element -- small static logo */}
                    <img className={"ico logo" + (node.round ? " round" : "")} src={node.logo} alt="" width={160} height={160} />
                    <div><h3>{node.name}</h3><p>{node.kind}</p></div>
                  </div>
                  <ul className="roles">
                    {node.roles.map((r) => (
                      <li key={r.title}><h4>{r.title}</h4><p>{r.text}{r.link && <> <Link href={r.link.href}>{r.link.label}</Link></>}</p></li>
                    ))}
                  </ul>
                </div>
              </article>
            </div>
          ))}
        </div>
        <div className="gend">
          <div className="gn" data-r>
            <p className="gl mono">{frameLabel(4)}</p>
            <div className="gb">
              <h3>What I keep coming back to</h3>
              <ul className="into">{interests.map((it) => <li key={it.label}><Glyph name={it.glyph} />{it.label}</li>)}</ul>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}
