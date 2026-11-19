import type { MetadataRoute } from "next";
import { projects } from "@/data/portfolio";
import { getPostCards } from "@/lib/posts";

export const dynamic = "force-static";

const SITE_URL = "https://khalilammar.me";

export default function sitemap(): MetadataRoute.Sitemap {
  const pages = ["/", "/blog/"].map((p) => ({ url: SITE_URL + p }));
  const work = projects.map((p) => ({ url: `${SITE_URL}/project/${p.slug}/` }));
  const posts = getPostCards().map((p) => ({ url: `${SITE_URL}/writeups/${p.id}/`, lastModified: p.date }));
  return [...pages, ...work, ...posts];
}
