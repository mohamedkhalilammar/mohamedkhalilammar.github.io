import type { Metadata, Viewport } from "next";
import { Bricolage_Grotesque, Geist, Geist_Mono } from "next/font/google";
import { Ambient } from "@/components/site/ambient";
import { Reveal } from "@/components/site/reveal";
import "./globals.css";
import "./pages.css";

const display = Bricolage_Grotesque({ variable: "--font-display", subsets: ["latin"], display: "swap" });
const body = Geist({ variable: "--font-body", subsets: ["latin"], display: "swap" });
const mono = Geist_Mono({ variable: "--font-mono", subsets: ["latin"], weight: ["400", "500"], display: "swap" });

const SITE_URL = "https://khalilammar.me";
const SITE_TITLE = "Khalil Ammar";
const SITE_DESCRIPTION =
  "The personal blog of Khalil Ammar (fadigaa), engineering student, cybersecurity enthusiast, and CTF player.";

export const metadata: Metadata = {
  metadataBase: new URL(SITE_URL),
  title: SITE_TITLE,
  description: SITE_DESCRIPTION,
  openGraph: { title: SITE_TITLE, description: SITE_DESCRIPTION, url: "/", siteName: "Khalil Ammar", type: "website" },
  twitter: { card: "summary_large_image", title: SITE_TITLE, description: SITE_DESCRIPTION },
};

export const viewport: Viewport = { themeColor: "#0D0B0A", colorScheme: "dark" };

// Runs before paint: force https on the live domain, and restore the motion switch so nothing animates for a frame first.
const BOOT = `
if (location.protocol !== 'https:' && location.hostname !== 'localhost' && location.hostname !== '127.0.0.1') location.replace('https://' + location.hostname + location.pathname + location.search);
try { var m = localStorage.getItem('lt-motion'); if (m === 'off' || (m !== 'on' && matchMedia('(prefers-reduced-motion: reduce)').matches)) document.documentElement.classList.add('still'); } catch (e) {}
`;

export default function RootLayout({ children }: Readonly<{ children: React.ReactNode }>) {
  return (
    <html lang="en" data-view="trace" suppressHydrationWarning className={`${display.variable} ${body.variable} ${mono.variable}`}>
      <head>
        <script dangerouslySetInnerHTML={{ __html: BOOT }} />
      </head>
      <body>
        <a className="skip" href="#main-content">Skip to content</a>
        <Ambient />
        <div className="page">{children}</div>
        <Reveal />
      </body>
    </html>
  );
}
