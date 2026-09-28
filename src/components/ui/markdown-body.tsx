import ReactMarkdown from "react-markdown";
import remarkGfm from "remark-gfm";
import type { Components } from "react-markdown";
import { CodeBlock } from "@/components/ui/code-block";
import type { Element, ElementContent } from "hast";
import { ArticleGallery, type ArticleImage } from "@/components/ui/article-gallery";

function collectImages(node: Element | ElementContent): ArticleImage[] {
  if (node.type !== "element") return [];
  if (node.tagName === "img" && typeof node.properties.src === "string") {
    return [{ src: node.properties.src, alt: String(node.properties.alt ?? "") }];
  }
  return node.children.flatMap(collectImages);
}

function onlyImages(node: Element | ElementContent): boolean {
  if (node.type === "text") return !node.value.trim();
  if (node.type !== "element") return false;
  return node.tagName === "img" || node.tagName === "br" ||
    (node.tagName === "a" && node.children.every(onlyImages));
}

export function articleHeadings(content: string) {
  let fence: string | null = null;
  return content.split("\n").flatMap((line, index) => {
    const delimiter = line.match(/^\s*(`{3,}|~{3,})/);
    if (delimiter) {
      if (!fence) fence = delimiter[1];
      else if (delimiter[1][0] === fence[0] && delimiter[1].length >= fence.length) fence = null;
      return [];
    }
    if (fence) return [];
    const heading = line.match(/^#{1,3}\s+(.+?)\s*#*$/);
    return heading ? [{ id: "section-" + (index + 1), text: heading[1].replace(/[*_`]/g, "") }] : [];
  });
}

const components: Components = {
  p: ({ node, ...props }) => node && node.children.every(onlyImages) && collectImages(node).length
    ? <ArticleGallery images={collectImages(node)} /> : <p {...props} />,
  h1: ({ node, ...props }) => <h2 id={"section-" + node?.position?.start.line} {...props} />,
  h2: ({ node, ...props }) => <h2 id={"section-" + node?.position?.start.line} {...props} />,
  h3: ({ node, ...props }) => <h3 id={"section-" + node?.position?.start.line} {...props} />,
  a: ({ node: _node, href, ...props }) => {
    const external = /^https?:\/\//.test(href ?? "");
    const media = /^\/media\//.test(href ?? "");
    return <a href={href} target={external || media ? "_blank" : undefined} rel={external || media ? "noopener noreferrer" : undefined} {...props} />;
  },
  table: ({ node, ...props }) => node && collectImages(node).length
    ? <ArticleGallery images={collectImages(node)} layout="row" />
    : <div className="article-table" role="region" aria-label="Article table" tabIndex={0}><table {...props} /></div>,
  pre: ({ node: _node, children }) => <CodeBlock>{children}</CodeBlock>,
  code: ({ node: _node, ...props }) => <code {...props} />,
  img: ({ node: _node, alt, ...props }) => <img alt={alt ?? ""} loading="lazy" {...props} />,
};

export function MarkdownBody({ children }: { children: string }) {
  return <div className="article-prose"><ReactMarkdown remarkPlugins={[remarkGfm]} components={components}>{children}</ReactMarkdown></div>;
}
