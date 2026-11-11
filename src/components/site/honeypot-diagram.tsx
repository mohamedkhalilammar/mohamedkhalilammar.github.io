/** Honeypot pipeline drawn as a flat diagram; colours come from the `.d-*` classes. */
export function HoneypotDiagram() {
  return (
    <figure className="diagram">
      <svg viewBox="0 0 720 330" role="img" aria-label="Pipeline: an attacker reaches Cowrie over SSH and FastAPI over HTTP. Both forward input to an orchestrator with a policy gate, which prompts an LLM and sends normalized telemetry to Wazuh.">
        <defs><marker id="ah" viewBox="0 0 10 10" refX="9" refY="5" markerWidth="7" markerHeight="7" orient="auto-start-reverse"><path className="d-arrow" d="M0 0 L10 5 L0 10 z" /></marker></defs>
        <rect className="d-box d-key" x="8" y="139" width="112" height="52" /><rect className="d-box" x="188" y="51" width="140" height="52" /><rect className="d-box" x="188" y="227" width="140" height="52" />
        <rect className="d-box d-key" x="388" y="139" width="150" height="52" /><rect className="d-box" x="592" y="51" width="120" height="52" /><rect className="d-box" x="592" y="227" width="120" height="52" />
        <path className="d-edge" d="M120 155 L150 155 L150 77 L186 77" markerEnd="url(#ah)" /><path className="d-edge" d="M120 175 L150 175 L150 253 L186 253" markerEnd="url(#ah)" />
        <path className="d-edge" d="M328 77 L358 77 L358 155 L386 155" markerEnd="url(#ah)" /><path className="d-edge" d="M328 253 L358 253 L358 175 L386 175" markerEnd="url(#ah)" />
        <path className="d-edge" d="M538 155 L564 155 L564 77 L590 77" markerEnd="url(#ah)" markerStart="url(#ah)" /><path className="d-edge" d="M538 175 L564 175 L564 253 L590 253" markerEnd="url(#ah)" />
        <g textAnchor="middle">
          <text className="d-t d-a" x="64" y="169">attacker</text><text className="d-t" x="258" y="74">Cowrie</text><text className="d-s" x="258" y="92">ssh</text>
          <text className="d-t" x="258" y="250">FastAPI</text><text className="d-s" x="258" y="268">http</text><text className="d-t" x="463" y="162">orchestrator</text><text className="d-s d-a" x="463" y="180">policy gate</text>
          <text className="d-t" x="652" y="82">LLM</text><text className="d-t" x="652" y="250">Wazuh</text><text className="d-s" x="652" y="268">siem</text>
        </g>
        <text className="d-s" x="388" y="126">; filtered before the model sees it</text><text className="d-s" x="478" y="312">; normalized telemetry</text>
      </svg>
    </figure>
  );
}
