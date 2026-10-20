import { certifications, learningPath } from "@/data/portfolio";
import Image from "next/image";

export function Credentials() {
  return (
    <div className="credentials">
      <div className="credential-grid">
        {certifications.map(cert => (
          <article className="credential" key={cert.name}>
            <div className="credential-heading">
              {cert.logo && <Image src={cert.logo} alt="" width={40} height={40} />}
              <div><p>{cert.issuer}</p><h3>{cert.name}</h3></div>
            </div>
            <details><summary>About this certification</summary><p>{cert.description}</p></details>
            <a href={cert.verifyUrl || cert.localFile} target="_blank" rel="noopener noreferrer">View certificate <span aria-hidden>↗</span></a>
          </article>
        ))}
      </div>
      <article className="credential credential-path">
        <div className="credential-heading">
          {learningPath.logo && <Image src={learningPath.logo} alt="" width={40} height={40} />}
          <div><p>{learningPath.provider} · {learningPath.status}</p><h3>{learningPath.name}</h3></div>
        </div>
        <a href={learningPath.localFile} target="_blank" rel="noopener noreferrer">View completion <span aria-hidden>↗</span></a>
        <details><summary>What the path covers</summary><p>{learningPath.summary}</p><ul>{learningPath.modules.map(module => <li key={module}>{module}</li>)}</ul></details>
      </article>
    </div>
  );
}
