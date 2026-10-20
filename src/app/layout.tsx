import type { Metadata } from "next";
import { Bricolage_Grotesque, Karla, Azeret_Mono, Syne } from "next/font/google";
import "./globals.css";
import "./refinements.css";
import "./portfolio-polish.css";

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
const SITE_TITLE = "Khalil Ammar";
const SITE_DESCRIPTION =
  "The personal blog of Khalil Ammar (fadigaa), engineering student, cybersecurity enthusiast, and CTF player.";

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
    // Image comes from ./opengraph-image.jpg via the file convention.
  },
  twitter: {
    card: "summary_large_image",
    title: SITE_TITLE,
    description: SITE_DESCRIPTION,
    // Image falls back to ./opengraph-image.jpg.
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
      data-theme="midnight"
      data-scroll-behavior="smooth"
      suppressHydrationWarning
      className={`${bricolage.variable} ${karla.variable} ${azeret.variable} ${syne.variable} h-full antialiased`}
    >
      <head>
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
        {children}
      </body>
    </html>
  );
}
