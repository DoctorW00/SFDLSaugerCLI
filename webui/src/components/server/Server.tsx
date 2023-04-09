import { IFile, IServer } from "../../api/IAppState";
import { FileList } from "./FileList";
import { NameValueList, NameValuePair, Table, Text } from "grommet";

export function Server(props: { server: IServer; files: IFile[] }) {
  console.log("Render server");
  return (
    <>
      <Table>
        <NameValueList>
          <NameValuePair name="Download Size">
            <Text color="text-strong">{props.server.total}</Text>
          </NameValuePair>
        </NameValueList>
      </Table>
      <FileList files={props.files} />
    </>
  );
}
