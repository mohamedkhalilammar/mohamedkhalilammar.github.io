import type { NextConfig } from "next";

const nextConfig: NextConfig = {
  output: "export",
  images: {
    unoptimized: true,
  },
  // If you are deploying to username.github.io/repo-name/
  // basePath: "/portfolio-amber", 
};

export default nextConfig;
