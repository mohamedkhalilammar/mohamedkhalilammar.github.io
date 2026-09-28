import PortfolioHome from "@/components/portfolio-home";
import { getPostCards } from "@/lib/posts";

export default function Home() {
  return <PortfolioHome posts={getPostCards()} />;
}
