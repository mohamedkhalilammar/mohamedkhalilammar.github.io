import { contactLead, credentials, profile } from "@/data/portfolio";
import { MapsLine } from "@/components/site/maps-line";

/** Contact: the process asks to continue. */
export function Contact() {
  return (
    <section className="contact" id="contact" aria-labelledby="contact-h">
      <div className="rv">
        <div className="head">
          <MapsLine region="contact" />
          <h2 className="h2" id="contact-h">Continue?</h2>
        </div>
        <p className="contact-lead">{contactLead}</p>
      </div>
      <div className="cgrid rv">
        <div data-r>
          <p className="kick" style={{ margin: 0 }}>Contact</p>
          <ul className="cl">
            <li><a href={"mailto:" + profile.socials.email}>{profile.socials.email}</a></li>
            <li><a href={profile.socials.github} target="_blank" rel="noopener noreferrer">GitHub</a></li>
            <li><a href={profile.socials.linkedin} target="_blank" rel="noopener noreferrer">LinkedIn</a></li>
          </ul>
        </div>
        <div data-r>
          <p className="kick" style={{ margin: 0 }}>Certifications</p>
          <ul className="cl certs">
            {credentials.map((c) => (
              <li key={c.name}>
                {/* eslint-disable-next-line @next/next/no-img-element -- static export */}
                <img src={c.logo} alt={c.issuer} width={c.width} height={c.height} loading="lazy" />
                <div>{c.name}<span>{c.issuer}</span></div>
              </li>
            ))}
          </ul>
        </div>
      </div>
    </section>
  );
}
