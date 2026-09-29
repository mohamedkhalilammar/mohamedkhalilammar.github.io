import type { Metadata } from "next";
import { Bricolage_Grotesque, Karla, Azeret_Mono, Syne } from "next/font/google";
import "./globals.css";
import "./refinements.css";
import { THEME_INIT_SCRIPT } from "@/lib/theme";
import { ThemeProvider } from "@/components/ui/theme-provider";

const bricolage = Bricolage_Grotesque({
  variable: "--font-heading",
  subsets: ["latin"],
  display: "swap",
});

// Expressive display face for hero titles — editorial, geometric, memorable
const syne = Syne({
  variable: "--font-display",
  subsets: ["latin"],
  weight: ["700", "800"],
  display: "swap",
});

const karla = Karla({
  variable: "--font-body",
  subsets: ["latin"],
  display: "swap",
});

const azeret = Azeret_Mono({
  variable: "--font-mono",
  subsets: ["latin"],
  display: "swap",
});

const SITE_URL = "https://khalilammar.me";
const SITE_TITLE = "Khalil Ammar | Security & Intelligence";
const SITE_DESCRIPTION =
  "Cybersecurity portfolio focused on reverse engineering, malware analysis, mobile pentesting, CTF performance, and practical offensive security projects.";

export const metadata: Metadata = {
  metadataBase: new URL(SITE_URL),
  title: SITE_TITLE,
  description: SITE_DESCRIPTION,
  openGraph: {
    title: SITE_TITLE,
    description: SITE_DESCRIPTION,
    url: "/",
    siteName: "Khalil Ammar",
    type: "website",
    // Image comes from ./opengraph-image.png via the file convention.
  },
  twitter: {
    card: "summary_large_image",
    title: SITE_TITLE,
    description: SITE_DESCRIPTION,
    // Image falls back to ./opengraph-image.png.
  },
};



export default function RootLayout({
  children,
}: Readonly<{
  children: React.ReactNode;
}>) {
  return (
    <html
      lang="en"
      data-scroll-behavior="smooth"
      suppressHydrationWarning
      className={`${bricolage.variable} ${karla.variable} ${azeret.variable} ${syne.variable} h-full antialiased`}
    >
      <head>
        {/* Must run before first paint, ahead of every other script, or a
            visitor on Amber Light gets a full-page flash of Midnight. */}
        <script dangerouslySetInnerHTML={{ __html: THEME_INIT_SCRIPT }} />
        <script
          dangerouslySetInnerHTML={{
            __html: `
              if (window.location.protocol !== 'https:' && window.location.hostname !== 'localhost' && window.location.hostname !== '127.0.0.1') {
                window.location.replace('https://' + window.location.hostname + window.location.pathname + window.location.search);
              }
            `,
          }}
        />
      </head>
      <body className="min-h-full flex flex-col font-body">
        <a className="skip-link" href="#main-content">Skip to content</a>
        <ThemeProvider>{children}</ThemeProvider>
      </body>
    </html>
  );
}
