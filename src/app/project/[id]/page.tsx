import { projects } from "@/data/portfolio";
import Link from "next/link";
import { notFound } from "next/navigation";
import fs from "fs";
import path from "path";
import ReactMarkdown from "react-markdown";
import rehypeRaw from "rehype-raw";

import { ProjectGallery } from "@/components/ui/project-gallery";
import { CinematicVideo } from "@/components/ui/cinematic-video";

export const dynamicParams = false;

export async function generateStaticParams() {
  const slugs = projects.map((p) => ({
    id: p.name.toLowerCase().trim().replace(/[^a-z0-9]+/g, "-").replace(/-+/g, "-").replace(/^-|-$/g, ""),
  }));
  return slugs;
}

export default async function ProjectPage({ params }: { params: Promise<{ id: string }> }) {
  const resolvedParams = await params;
  const id = resolvedParams.id;

  const project = projects.find(
    (p) => p.name.toLowerCase().trim().replace(/[^a-z0-9]+/g, "-").replace(/-+/g, "-").replace(/^-|-$/g, "") === id
  );

  if (!project) {
    notFound();
  }

  const stackPreview = project.stack.slice(0, 4).join(" · ");
  const projectPulse = [
    { label: "Role", value: project.role || "Security Engineer" },
    { label: "Timeline", value: project.duration || "Continuous" },
    { label: "Stack Depth", value: `${project.stack.length} technologies` },
    { label: "Focus", value: project.context || "Applied Security Engineering" },
  ];

  let markdownContent = null;
  if (project.detailsFile) {
    try {
      const filePath = path.join(process.cwd(), "public", project.detailsFile);
      markdownContent = fs.readFileSync(filePath, "utf-8");
    } catch (e) {
      console.error(`Failed to read markdown file for ${project.name}`, e);
    }
  }
  const getSlug = (name: string) => name.toLowerCase().trim().replace(/[^a-z0-9]+/g, "-").replace(/-+/g, "-").replace(/^-|-$/g, "");
  const currentIndex = projects.findIndex(p => getSlug(p.name) === id);
  const nextProject = projects[(currentIndex + 1) % projects.length];
  const nextSlug = getSlug(nextProject.name);

  return (
    <div className="page-shell bg-[#252526] min-h-screen">
      {/* Simplified Space */}
      <div className="fixed left-0 top-0 bottom-0 w-[1px] bg-white/[0.03] z-50" />

      <header className="fixed top-0 left-0 right-0 z-50 px-6 py-6 md:px-12 flex justify-between items-center pointer-events-none">
        <Link 
          href="/#projects" 
          className="group flex items-center gap-3 px-6 py-3 rounded-xl border border-white/5 bg-white/[0.01] hover:bg-white/[0.04] hover:border-amber-500/30 transition-all duration-500 backdrop-blur-sm pointer-events-auto shadow-2xl"
        >
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="3" strokeLinecap="round" strokeLinejoin="round" className="text-amber-500 group-hover:-translate-x-1.5 transition-transform duration-500">
            <path d="M19 12H5M12 19l-7-7 7-7" />
          </svg>
          <span className="font-mono text-[10px] uppercase font-bold tracking-[0.3em] text-white/40 group-hover:text-white transition-colors">
            Back to Catalog
          </span>
        </Link>

        <Link 
          href={`/project/${nextSlug}`} 
          className="group flex items-center gap-3 px-6 py-3 rounded-xl border border-white/5 bg-white/[0.01] hover:bg-white/[0.04] hover:border-amber-500/30 transition-all duration-500 backdrop-blur-sm pointer-events-auto shadow-2xl"
        >
          <span className="font-mono text-[10px] uppercase font-bold tracking-[0.3em] text-white/40 group-hover:text-white transition-colors">
            Next Project
          </span>
          <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="3" strokeLinecap="round" strokeLinejoin="round" className="text-amber-500 group-hover:translate-x-1.5 transition-transform duration-500">
            <path d="M5 12h14M12 5l7 7-7 7" />
          </svg>
        </Link>
      </header>

      <main className="max-w-[1300px] mx-auto px-6 md:px-12 lg:px-20" style={{ paddingTop: "8.5rem", paddingBottom: "10rem" }}>

        {/* TOP LEVEL INTEL: Title & Summary */}
        <div className="relative mb-24 css-stagger-item">
          {/* Decorative Corner Brackets */}
          <div className="absolute -top-12 -left-12 w-24 h-24 border-t border-l border-white/10 rounded-tl-[3rem]" />

          <div className="flex flex-col gap-6">
            <div className="flex items-center gap-4">
              <div className="px-3 py-1 rounded bg-white/5 border border-white/10 text-[10px] font-mono font-bold text-amber-500 uppercase tracking-[0.3em]">
                Project Overview
              </div>
              <div className="h-px w-24 bg-white/10" />
              <p className="font-mono text-xs uppercase tracking-[0.4em] text-white/20 font-bold">{project.context || "Engineering Student"}</p>
            </div>

            <h1 className="font-sans text-5xl md:text-7xl lg:text-8xl font-black text-white uppercase tracking-tighter leading-[0.85] mb-8 drop-shadow-2xl">
              {project.name.split(" ").map((word, i) => (
                <span key={i} className={i % 2 !== 0 ? "text-amber-500" : ""}>
                  {word}{" "}
                </span>
              ))}
            </h1>

            <p className="text-lg md:text-2xl leading-relaxed text-zinc-400 max-w-4xl font-medium">
              {project.summary}
            </p>
          </div>
        </div>


        {/* CORE CONTENT LAYOUT */}
        <div className="grid grid-cols-1 lg:grid-cols-12 gap-16">

          <div className="lg:col-span-7 space-y-32">

            {/* Visual Evidence Area */}
            {(project.mediaUrl || (project.screenshots && project.screenshots.length > 0)) && (
              <div className="css-stagger-item">
                <div className="flex items-center gap-6 mb-8">
                  <h2 className="font-sans text-2xl font-black uppercase tracking-tighter text-white">Visual Architecture</h2>
                  <div className="h-px flex-1 bg-white/[0.05]" />
                </div>

                {/* Enhanced Media Frame */}
                {project.mediaUrl && (
                  <div className="relative rounded-[2rem] overflow-hidden border border-white/10 bg-black shadow-2xl mb-16">
                    <div className="w-full">
                      {project.mediaUrl.includes("youtube.com/embed") ? (
                        <CinematicVideo
                          url={project.mediaUrl}
                          title={`${project.name} Video Demo`}
                        />
                      ) : project.mediaUrl.match(/\.(mp4|webm|ogg)$/i) ? (
                        <video src={project.mediaUrl} controls autoPlay muted loop playsInline className="w-full h-auto aspect-video object-cover" />
                      ) : (
                        <img src={project.mediaUrl} alt="Preview" className="w-full h-auto aspect-video object-cover" />
                      )}
                    </div>
                  </div>
                )}

                {/* Screenshot Gallery */}
                {project.screenshots && project.screenshots.length > 0 && (
                  <ProjectGallery name={project.name} screenshots={project.screenshots} screenshotCaptions={project.screenshotCaptions} />
                )}
              </div>
            )}

            {/* Deep Analysis (Descriptions) - Tactical Grid Upgrade */}
            <div className="css-stagger-item">
              <div className="grid grid-cols-1 md:grid-cols-2 gap-8 items-start py-12 border-t border-white/5">
                {[
                  { title: "System Architecture", content: project.architecture },
                  { title: "Implementation Strategy", content: project.challenges },
                  { title: "Technical Outcome", content: project.impact }
                ].filter(s => s.content).map((section, idx) => (
                  <div key={idx} className="group p-8 rounded-2xl bg-white/[0.015] border border-white/5 hover:border-amber-500/30 transition-all duration-500 relative overflow-hidden h-full"> 
                    <div className="absolute top-0 right-0 p-4 font-mono text-3xl font-black text-white/[0.02] group-hover:text-amber-500/10 italic select-none">
                      0{idx + 1}
                    </div>
                    <h2 className="font-sans text-xl font-black uppercase tracking-tighter text-amber-500 mb-6 flex items-center gap-3">
                      <span className="w-2 h-2 rounded-full bg-amber-500/50" />
                      {section.title}
                    </h2>
                    <p className="text-lg leading-relaxed text-zinc-400 font-medium group-hover:text-zinc-200 transition-colors">
                      {section.content}
                    </p>
                  </div>
                ))}
              </div>

              {project.features && (
                <div className="mt-12 p-10 md:p-12 rounded-[2rem] bg-white/[0.02] border border-white/[0.06] relative overflow-hidden group">
                  <div className="absolute top-0 right-0 w-48 h-48 bg-amber-500/5 blur-[80px] group-hover:bg-amber-500/10 transition-all" />
                  <h2 className="font-sans text-3xl font-black uppercase tracking-tighter text-white mb-10">System Features</h2>
                  <div className="grid grid-cols-1 md:grid-cols-2 gap-x-12 gap-y-6">
                    {project.features.map((feature, i) => (
                      <div key={feature} className="flex items-center gap-4 group/item">
                        <div className="w-1.5 h-1.5 rounded-full bg-amber-500/30 group-hover/item:bg-amber-500 transition-all" />
                        <span className="text-zinc-400 font-medium group-hover/item:text-white transition-colors">{feature}</span>
                      </div>
                    ))}
                  </div>
                </div>
              )}
            </div>
          </div>

          {/* SIDEBAR: Technical Profile */}
          <aside className="lg:col-span-5 lg:sticky lg:top-32 self-start space-y-10 css-stagger-item">
            <div className="p-10 rounded-3xl bg-[#333333] border border-white/10 relative overflow-hidden">
              {/* Background Grid Accent */}
              <div className="absolute inset-0 opacity-[0.03] pointer-events-none" style={{ backgroundImage: "radial-gradient(var(--accent) 1px, transparent 0)", backgroundSize: "20px 20px" }} />

              <h3 className="font-sans text-xl font-bold text-white uppercase mb-10 pb-6 border-b border-white/5 flex items-center justify-between">
                Technical Profile
              </h3>

              <div className="space-y-8">

                <div className="space-y-6 pt-6 border-t border-white/5">
                  <p className="font-mono text-[9px] uppercase tracking-[0.3em] text-white/30">Technology Array</p>
                  <ul className="grid grid-cols-1 sm:grid-cols-2 gap-3">
                    {project.stack.map(tech => (
                      <li key={tech} className="flex items-center gap-3 group/li">
                        <div className="w-1.5 h-1.5 rounded-full bg-amber-500/20 group-hover/li:bg-amber-500 group-hover/li:scale-150 transition-all" />
                        <span className="font-mono text-xs text-white/60 group-hover/li:text-white transition-colors tracking-wide">{tech}</span>
                      </li>
                    ))}
                  </ul>
                </div>
              </div>

              <div className="mt-12">
                <a
                  href={project.githubUrl || "https://github.com/khalilammarr"}
                  target="_blank"
                  rel="noopener noreferrer"
                  className="btn-primary w-full flex items-center justify-center gap-4 py-5 px-10"
                >
                  <svg className="w-6 h-6 flex-shrink-0" fill="currentColor" viewBox="0 0 24 24">
                    <path d="M12 0c-6.626 0-12 5.373-12 12 0 5.302 3.438 9.8 8.207 11.387.599.111.793-.261.793-.577v-2.234c-3.338.726-4.033-1.416-4.033-1.416-.546-1.387-1.333-1.756-1.333-1.756-1.089-.745.083-.729.083-.729 1.205.084 1.839 1.237 1.839 1.237 1.07 1.834 2.807 1.304 3.492.997.107-.775.418-1.305.762-1.604-2.665-.305-5.467-1.334-5.467-5.931 0-1.311.469-2.381 1.236-3.221-.124-.303-.535-1.524.117-3.176 0 0 1.008-.322 3.301 1.23.957-.266 1.983-.399 3.003-.404 1.02.005 2.047.138 3.006.404 2.291-1.552 3.297-1.23 3.297-1.23.653 1.653.242 2.874.118 3.176.77.84 1.235 1.911 1.235 3.221 0 4.609-2.807 5.624-5.479 5.921.43.372.823 1.102.823 2.222v3.293c0 .319.192.694.801.576 4.765-1.589 8.199-6.086 8.199-11.386 0-6.627-5.373-12-12-12z" />
                  </svg>
                  <span className="whitespace-nowrap">Visit GitHub</span>
                </a>
              </div>
            </div>

          </aside>
        </div>

        {/* FULL WIDTH MARKDOWN OR CUSTOM STRUCTURED CONTENT */}
        {markdownContent && (
          <div className="mt-32 w-full relative">
            {project.name.toLowerCase().includes("honeypot") ? (
              <div className="space-y-32">
                {/* Tactical Deployment Roadmap */}
                <div className="grid grid-cols-1 md:grid-cols-3 gap-8">
                  {[
                    { step: "01", title: "Clone & Initialize", cmd: "git clone https://github.com/khalilammarr/LLM_Honeypot.git", desc: "Retrieve the core deception engine and SIEM infrastructure." },
                    { step: "02", title: "Cognitive Config", cmd: "cp .env.example .env && nano .env", desc: "Inject Groq API keys and define behavioral security policies." },
                    { step: "03", title: "Containerized Orchestration", cmd: "docker compose up -d --build", desc: "Spin up the Cowrie, FastAPI, and Wazuh stack in isolated zones." }
                  ].map((item, i) => (
                    <div key={i} className="group p-8 rounded-3xl bg-white/[0.02] border border-white/5 hover:border-amber-500/30 transition-all duration-500 relative overflow-hidden">
                      <div className="absolute top-0 right-0 p-4 font-mono text-4xl font-black text-white/[0.02] group-hover:text-amber-500/10 transition-colors uppercase italic">{item.step}</div>
                      <div className="relative z-10">
                        <h4 className="font-sans text-lg font-bold text-white mb-4 uppercase tracking-tighter">{item.title}</h4>
                        <div className="bg-[#0c1117] rounded-xl overflow-hidden border border-white/10 shadow-inner mb-6 group/code transition-all duration-300 hover:border-amber-500/20">
                          <div className="bg-white/5 border-b border-white/5 px-4 py-2 flex items-center justify-between">
                            <div className="flex gap-1.5">
                              <div className="w-1.5 h-1.5 rounded-full bg-zinc-800" />
                              <div className="w-1.5 h-1.5 rounded-full bg-zinc-800" />
                              <div className="w-1.5 h-1.5 rounded-full bg-zinc-800" />
                            </div>
                            <div className="flex items-center gap-2">
                              <svg width="10" height="10" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5" className="text-amber-500/40">
                                <polyline points="4 17 10 11 4 5" /><line x1="12" y1="19" x2="20" y2="19" />
                              </svg>
                              <span className="font-mono text-[8px] uppercase tracking-widest text-white/20 font-bold">bash://terminal</span>
                            </div>
                          </div>
                          <div className="p-5 font-mono text-[12px] md:text-[13px] leading-relaxed relative flex items-start group-hover/code:bg-white/[0.01] transition-colors">
                            <span className="text-amber-500/40 mr-4 select-none mt-1">$</span>
                            <code className="text-zinc-200 block break-all selection:bg-amber-500/30">
                              {item.cmd.split(' ').map((word, i) => (
                                <span key={i} className={i === 0 ? "text-amber-500 font-bold" : "text-zinc-300"}>
                                  {word}{" "}
                                </span>
                              ))}
                            </code>
                          </div>
                        </div>
                        <p className="text-sm text-zinc-500 leading-relaxed font-medium">{item.desc}</p>
                      </div>
                    </div>
                  ))}
                </div>

                {/* System Capabilities Matrix */}
                <div className="p-12 md:p-20 rounded-[3rem] bg-gradient-to-br from-white/[0.03] to-transparent border border-white/10 relative overflow-hidden">
                  <div className="absolute top-0 left-0 w-full h-full bg-[url('https://www.transparenttextures.com/patterns/carbon-fibre.png')] opacity-[0.03] pointer-events-none" />
                  
                  <div className="relative z-10 max-w-5xl">
                    <h2 className="font-sans text-4xl md:text-6xl font-black text-white uppercase tracking-tighter mb-16 px-2 border-l-4 border-amber-500">
                      System Capabilities
                    </h2>
                    
                    <div className="grid grid-cols-1 md:grid-cols-2 gap-12">
                      {[
                        { title: "Cognitive SSH", desc: "Cowrie-based shell that hallucinates entire file systems and command responses in real-time via LLM reasoning." },
                        { title: "Dynamic HTTP Surface", desc: "FastAPI-driven adaptive portal that generates realistic config files and database dumps on the fly." },
                        { title: "Zero-Leak Gating", desc: "Custom Python filters that strip model-specific reasoning tags to maintain full environmental immersion." },
                        { title: "SIEM Intelligence", desc: "Native Wazuh integration for real-time telemetry ingestion and automated adversarial behavioral analysis." }
                      ].map((cap, i) => (
                        <div key={i} className="group space-y-4">
                          <h4 className="font-sans text-xl font-black text-white uppercase tracking-tighter flex items-center gap-4">
                            <span className="w-2 h-2 bg-amber-500 rounded-full" />
                            {cap.title}
                          </h4>
                          <p className="text-zinc-400 leading-relaxed font-medium pl-6 border-l border-white/5 group-hover:border-amber-500/30 transition-colors">
                            {cap.desc}
                          </p>
                        </div>
                      ))}
                    </div>
                  </div>
                </div>

                {/* Safety Protocol */}
                <div className="p-8 rounded-2xl bg-amber-500/5 border border-amber-500/20 flex flex-col md:flex-row gap-8 items-center md:items-start max-w-4xl mx-auto">
                   <div className="w-16 h-16 rounded-full bg-amber-500/20 flex items-center justify-center shrink-0">
                      <svg width="24" height="24" viewBox="0 0 24 24" fill="none" stroke="currentColor" strokeWidth="2.5" className="text-amber-500">
                        <path d="M10.29 3.86L1.82 18a2 2 0 0 0 1.71 3h16.94a2 2 0 0 0 1.71-3L13.71 3.86a2 2 0 0 0-3.42 0zM12 9v4M12 17h.01" />
                      </svg>
                   </div>
                   <div>
                     <h4 className="font-mono text-xs font-black uppercase tracking-widest text-amber-500 mb-2">Security Advisory</h4>
                     <p className="text-sm text-amber-500/80 font-medium leading-relaxed">
                        This is an active deception lab platform. These services should only be hosted in strictly firewalled or host-only environments. Do not deploy on the public internet without proper sandboxing or isolation protocols.
                     </p>
                   </div>
                </div>
              </div>
            ) : (
              <div className="project-markdown-block p-8 md:p-16 border border-white/5 bg-gradient-to-b from-white/[0.02] to-transparent rounded-[3rem] w-full relative overflow-hidden">
                <div className="absolute top-0 right-0 w-96 h-96 bg-amber-500/5 blur-[120px] pointer-events-none" />
                <div className="absolute bottom-0 left-0 w-64 h-64 bg-amber-500/5 blur-[100px] pointer-events-none" />
                <div className="prose prose-invert prose-amber prose-lg leading-relaxed max-w-none relative z-10">
                  <ReactMarkdown rehypePlugins={[rehypeRaw]}>{markdownContent}</ReactMarkdown>
                </div>
              </div>
            )}
          </div>
        )}

        {/* NEXT PROJECT CTA */}
        <div className="mt-40 pt-20 border-t border-white/5 flex flex-col items-center text-center">
          <p className="font-mono text-[10px] uppercase tracking-[0.4em] text-amber-500/60 mb-6">Continue Exploration</p>
          <Link 
            href={`/project/${nextSlug}`}
            className="group relative flex flex-col items-center"
          >
            <h3 className="font-sans text-4xl md:text-6xl font-black text-white/40 group-hover:text-amber-500 transition-all duration-500 uppercase tracking-tighter">
              Next Project
            </h3>
            <div className="flex items-center gap-4 mt-4">
              <span className="h-px w-12 bg-white/10 group-hover:bg-amber-500/50 transition-all" />
              <span className="font-sans text-xl md:text-2xl font-bold text-white group-hover:scale-110 transition-transform duration-500">
                {nextProject.name}
              </span>
              <span className="h-px w-12 bg-white/10 group-hover:bg-amber-500/50 transition-all" />
            </div>
          </Link>
        </div>
      </main>
    </div>
  );
}

