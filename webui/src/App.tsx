import React, { useEffect, useState } from "react";
import {
  Header,
  Button,
  Grommet,
  Menu,
  Page,
  PageContent,
  PageHeader,
  Anchor,
  Tabs,
  Tab,
} from "grommet";
import { Home, User } from "grommet-icons";
import { connect } from "./api/ServerCommunication";

import { useSetState, useTrackedState } from "./api/AppState";
import { IAppState } from "./api/IAppState";
import { Server } from "./components/server/Server";
import { SFDLModal } from "./components/SFDLModal";
import { theme } from "./components/theme";

function App() {
  const setState = useSetState();
  const state = useTrackedState();

  const [connection, setConnection] = useState<WebSocket | undefined>(
    undefined
  );

  const [showAddSFDLModal, setShowAddSFDLModal] = useState(true);

  // Setup connection + Update global app state
  useEffect(() => {
    const [socket, cleanup] = connect();

    socket.addEventListener("message", ({ data }) => {
      setState(JSON.parse(data) as IAppState);
    });

    setConnection(socket);
    return cleanup;
  }, []);

  if (!state) {
    // TODO: Loading spinner
    return <>...</>;
  }

  return (
    <Grommet full theme={theme}>
      {showAddSFDLModal && (
        <SFDLModal close={() => setShowAddSFDLModal(false)} />
      )}
      <Page>
        <PageContent border={{ side: "bottom" }}>
          <Header
            align="center"
            direction="row"
            flex={false}
            justify="between"
            gap="medium"
          >
            <Anchor label="SFDL Sauger" icon={<Home />} />
            <Menu icon={<User />} items={[{ label: "sign out" }]} />
          </Header>
        </PageContent>
        <PageContent align="stretch">
          <PageHeader title="Downloads" margin={{ vertical: "medium" }} />
          <Header
            align="center"
            direction="row"
            flex={false}
            justify="end"
            gap="medium"
            margin={{ bottom: "small" }}
          >
            {/*<Box align="center" justify="center" direction="row">*/}
            {/*  <TextInput icon={<Search />} reverse />*/}
            {/*  <Button icon={<Filter />} tip="filter" />*/}
            {/*</Box>*/}
            <Button label="Add SFDL" onClick={() => {}} />
          </Header>
          <Tabs>
            {state.servers.length === 0 && <Tab />}
            {state.servers.map((s) => (
              <Tab title={s.name} key={s.id}>
                <Server
                  server={s}
                  files={state?.files.filter((f) => f.dServerID == s.id)}
                />
              </Tab>
            ))}
          </Tabs>
        </PageContent>
      </Page>
    </Grommet>
  );
}

export default App;
