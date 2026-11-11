import type { ReactNode } from "react";
import { MapsLine } from "@/components/site/maps-line";
import type { SectionId } from "@/lib/trace";

type Props = { region: SectionId; id: string; title: string; lede?: ReactNode; name?: string; as?: "h1" | "h2" };

/** Section opener: maps line, big heading rising out of a clip, lede underneath. */
export function SectionHead({ region, id, title, lede, name, as: Tag = "h2" }: Props) {
  return (
    <div className="intro rv">
      <div className="head">
        <MapsLine region={region} name={name} />
        <Tag className="h2" id={id}>{title}</Tag>
      </div>
      {lede && <p className="lede">{lede}</p>}
    </div>
  );
}
