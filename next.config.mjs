/** @type {import('next').NextConfig} */
const nextConfig = {
  output: "export",
  // Emit blog/index.html so both /blog and /blog/ resolve on GitHub Pages.
  trailingSlash: true,
  allowedDevOrigins: ["localhost", "127.0.0.1"],
  images: {
    unoptimized: true,
  },
};

export default nextConfig;
