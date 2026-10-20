"use client";

import { useRef, useState, type ReactNode } from "react";

export function CodeBlock({ children, language }: { children: ReactNode; language?: string }) {
  const ref = useRef<HTMLPreElement>(null);
  const [status, setStatus] = useState<"idle" | "copied" | "failed">("idle");
  const copy = async () => {
    try {
      await navigator.clipboard.writeText(ref.current?.textContent ?? "");
      setStatus("copied");
    } catch {
      setStatus("failed");
    }
  };
  return (
    <div className="article-code">
      <div className="article-code-toolbar">
        <span className="article-code-controls" aria-hidden="true"><i /><i /><i /></span>
        <span className="article-code-language">{language || "text"}</span>
        <button type="button" onClick={copy} aria-label={status === "copied" ? "Code copied" : "Copy code"}>
          {status === "copied" ? "Copied" : <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="1.6" aria-hidden="true"><rect x="8" y="8" width="12" height="13" rx="2" /><path d="M16 8V5a2 2 0 0 0-2-2H5a2 2 0 0 0-2 2v9a2 2 0 0 0 2 2h3" /></svg>}
        </button>
        <span role="status" className={status === "failed" ? "copy-error" : "sr-only"}>{status === "failed" ? "Couldn't copy. Select the code to copy manually." : status === "copied" ? "Code copied to clipboard." : ""}</span>
      </div>
      <pre ref={ref} tabIndex={0}>{children}</pre>
    </div>
  );
}
