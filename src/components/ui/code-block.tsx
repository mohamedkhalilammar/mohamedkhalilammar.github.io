"use client";

import { useRef, useState, type ReactNode } from "react";

export function CodeBlock({ children }: { children: ReactNode }) {
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
      <div className="article-code-toolbar"><span>Code</span><button type="button" onClick={copy}>{status === "copied" ? "Copied" : "Copy code"}</button><span role="status" className={status === "failed" ? "copy-error" : "sr-only"}>{status === "failed" ? "Couldn't copy. Select the code to copy manually." : status === "copied" ? "Code copied to clipboard." : ""}</span></div>
      <pre ref={ref} tabIndex={0}>{children}</pre>
    </div>
  );
}
